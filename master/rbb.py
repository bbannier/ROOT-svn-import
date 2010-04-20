# ROOT BuildBot tools

import re

from buildbot import scheduler, locks
from buildbot.buildslave import BuildSlave
from buildbot.changes.svnpoller import SVNPoller
from buildbot.process import factory
from buildbot.scheduler import Scheduler, Try_Jobdir
from buildbot.status import html
from buildbot.status.builder import SUCCESS, WARNINGS, FAILURE, SKIPPED, \
    EXCEPTION
from buildbot.status.mail import MailNotifier
from buildbot.status.web.auth import HTPasswdAuth
from buildbot.steps import trigger
from buildbot.steps.shell import ShellCommand
from buildbot.steps.source import SVN

class ROOTBuildSource:
    """A set of sources (ROOT, roottest) plus builders."""

    name = ""
    svnurl = ""
    svncheckurl = ""
    triggeredby = ""

    def __init__(self, name, make, svnurl, svncheckurl, viewvc, triggeredby,
                 configure, configFlags, envScript, schedule):
        self.name = name
        self.make = make
        self.svnurl = svnurl
        self.svncheckurl = svncheckurl
        self.viewvc = viewvc
        self.triggeredby = triggeredby
        self.triggers = []
        self.configure = configure
        self.configFlags = configFlags
        self.envScript = envScript
        self.schedule = schedule

    def addTrigger(self, trig):
        self.triggers.append(trig)

    def fileIsImportant(self, change):
        # check whether the Change is important:
        for name in change.files:
            if name.find('/doc/') != -1: continue
            if name.find('/README/') != -1: continue
            if name.find('/tutorials/') != -1: continue
            if name.find('/build/package/') != -1: continue
            if name.find('/build/misc/') != -1: continue
            if name.find('/etc/') != -1: continue
            if name.find('/fonts/') != -1: continue
            if name.find('/icons/') != -1: continue
            if name.find('/macros/') != -1: continue
            if name.find('/man/') != -1: continue
            if name.endswith('.html'): continue
            if name.endswith('.txt'): continue
            return True
        return False

class ROOTTestCmd(ShellCommand):
    name = "roottest"
    haltOnFailure = 1
    flunkOnFailure = 1
    description = ["running roottest"]
    descriptionDone = ["roottest"]

    def __init__(self, workdir=None, **kwargs):
        ShellCommand.__init__(self, workdir, **kwargs)
        self.makere      = re.compile(r'make\[\d+\]: \*\*\* \[([^]]+)')
        self.ignoreoutre = re.compile(r'^(Known failures:)|(Warning [A-Za-z])|(ROOT .* does not support)')
        self.ignoreerrre = re.compile(r'^Target `test. not remade because of errors\.')
        self.ignoreother = re.compile(r'^(Running test|Tests) in ')
        self.failureDescr = []
        self.numFailures = 0

    def getFailedDirLog(self, faileddir, loglines, end):
        ret = []
        foundStart = False
        skipSubDir = None
        runningTestInMe = 'Running test in ' + faileddir
        lenRunningTestInMe = len(runningTestInMe)
        outSoFar = ""
        for line in loglines:
            if line == end: break

            sline = line.strip()
            haveRunningTestInMe = sline.find(runningTestInMe)
            if not foundStart:
                if haveRunningTestInMe != -1: foundStart = True
                continue
            if skipSubDir:
                if 'Tests in '+ skipSubDir in sline: skipSubDir = None
                continue
            if haveRunningTestInMe != -1:
                subdir = sline[haveRunningTestInMe + lenRunningTestInMe + 1:]
                if len(subdir):
                    skipSubDir = subdir
                    continue
                continue
            if self.ignoreother.search(sline): continue
            smakere = self.makere.search(sline)
            if smakere:
                testname = smakere.groups()[0]
                ret.append({'target' : testname.strip("'"), 'log': outSoFar})
                outSoFar = ""
                continue
            if self.ignoreoutre.search(sline) or self.ignoreerrre.search(sline):
                continue
            outSoFar += line + '\n'

        if len(ret) == 0: return None

        return {'dir': faileddir.strip("'"), 'tests' : ret}
            

    def createSummary(self, logfile):
        failre      = re.compile(r'^Test in (\S+) \.+ FAIL')

        failedDirLogs = []
        self.failureDescr = []

        # This signals a failure of ABC in xyz:
        #   make[n]: *** [ABC] Error m
        #   Test in ./xyz ... FAIL
        # We cannot reliably tell what output belongs to this test, because
        # roottest can be run with -j24, so we need to include everything between
        #   Running test in ./xyz
        # and
        #   make [n]: *** [ABC] Error m

        alllines =  logfile.getText().split("\n")

        for line in reversed(alllines):
            sline = line.strip()
            sfailre = failre.search(sline)
            if sfailre:
                dirname = sfailre.groups()[0]
                failedLog = self.getFailedDirLog(dirname, alllines, line)
                if failedLog:
                    self.numFailures += len(failedLog['tests'])
                    for test in failedLog['tests']:
                        target = test['target']
                        if target[0:2] == './': target = target[2:]
                        self.failureDescr.append(dirname + ': ' + target)
                    failedDirLogs.append(failedLog)

        if self.numFailures > 0:
            self.addHTMLLog("failures", self.formatFailures(failedDirLogs))
            if self.numFailures > 3: self.failureDescr = ['(too many)' ]

    def formatFailures(self, failures):
        html = '''<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html
 xmlns="http://www.w3.org/1999/xhtml"
 lang="en"
 xml:lang="en">
<head>'''
        html += '<title>' + str(self.describe(True)) + ''' failure log</title>
<link href="buildbot.css" rel="stylesheet" type="text/css" />
<style type="text/css">
 div.directory {
  margin: 1em;
  padding: 1em 0 0 1em;
  border-left: 1em solid #b33;
  border-bottom: 0.1em solid #b33;
  border-top: 0.1em solid #b33;
  border-right: 0.1em solid #b33;
 }
 span.directory {
  font-size: x-large;
  font-weight: bold;
  color: #00c;
 }
 div.test {
  padding: 0.5em 0 0 0.5em;
 }
 span.test {
  font-size: larger;
  color: #00c;
 }
 pre {
  font-size: smaller;
  margin-top: 0.3em;
  padding: 1em;
  font-family: "Courier New", courier, monotype;
  background-color: #f0f0ff;
 }
</style>
</head><body>'''
        for failedLog in failures:
            faildir = failedLog['dir'].strip("'")
            html += '<div class="directory"><span class="directory">' + faildir + '</span>\n'
            failedtests = failedLog['tests']
            for failedtest in failedtests:
                html += '<div class="test"><span class="test">' + failedtest['target'] + '</span>\n'
                html += '<pre>\n' + failedtest['log'] + '</pre></div>'
            html += '</div>'
        return html

    def getText(self, cmd, results):
        if results == SUCCESS:
            return self.describe(True)
        elif results == WARNINGS:
            return self.describe(True) + ["warnings"]
        else:
            if self.numFailures > 0:
                plural = ''
                if self.numFailures > 1: plural = 's'
                return self.describe(True) + ["%s failure%s:" % (self.numFailures, plural)] + self.failureDescr
            else :
                return self.describe(True) + ["failed"]

class ROOTSVN(SVN):
    def __init__(self, category="ROOT", svnurl=None, **kwargs):
        self.category = category
        SVN.__init__(self, svnurl=svnurl, **kwargs)

    def computeSourceRevision(self, changes):
        # Find the latest change matching our category
        changes = filter(lambda c: c.category == self.category, changes)
        if not changes or None in [c.revision for c in changes]:
            return None
        lastChange = max([int(c.revision) for c in changes])
        return lastChange

    def start(self):
        # Use HEAD if the build's revision does not belong to our category.
        forcedHead = False
        # determine sourcestamp's revision as used by SVN.start():
        if not self.alwaysUseLatest:
            s = self.build.getSourceStamp()
            revision = s.revision
            # match that number with changes' revisions:
            categoryChanges = filter(lambda c: c.revision == revision and c.category == self.category, s.changes)
            if not categoryChanges:
                # there was no change with that revision number matching our category, so go to HEAD:
                forcedHead = True
                self.alwaysUseLatest = True
        # invoke base class's start():
        ret = SVN.start(self)
        if forcedHead: self.alwaysUseLatest = False
        return ret

class ROOTMailNotifier(MailNotifier):
    """Overrides when to send emails (only first failing build,
       only on new roottest failures)"""
    def __init__(self):
        MailNotifier.__init__(self, fromaddr="buildbot@root.cern.ch",
                              lookup="root.cern.ch",
                              extraRecipients=['rootsvn@root.cern.ch'],
                              mode="problem",
                              addPatch=False,
                              categories=['ROOT-incr','roottest-incr','ROOT-hourly','roottest-hourly'],
                              messageFormatter = self.messageFormatter
                              )
        # Sent email for these revisions (previous 10)
        self.notifiedRevisions = list()

    def buildFinished(self, name, build, results):
        # Near-copy from MailNotifier.buildFinished()
        # here is where we actually do something.
        builder = build.getBuilder()
        if self.builders is not None and name not in self.builders:
            return # ignore this build
        if self.categories is not None and \
               builder.category not in self.categories:
            return # ignore this build

        if results != FAILURE:
            return

        if build.getSourceStamp() and build.getSourceStamp().revision:
            revision = build.getSourceStamp().revision
            if revision in self.notifiedRevisions:
                return

            prev = build.getPreviousBuild()
            if prev and prev.getResults() == FAILURE:
                # We might have the same failures; compare the failing steps' names.
                if self.getSteps() \
                        and len(prev.getSteps()) == len(self.getSteps()) \
                        and ( prev.getSteps()[-1].getText() == self.getSteps()[-1].getText() \
                                  and name == "roottest"):
                    # Looks like a similar roottest failure.
                    # Assume it's still caused by the same error.
                    return

            self.notifiedRevisions.push(revision)
            if len(self.notifiedRevisions) > 30: self.notifiedRevisions.pop()

        # for testing purposes, buildMessage returns a Deferred that fires
        # when the mail has been sent. To help unit tests, we return that
        # Deferred here even though the normal IStatusReceiver.buildFinished
        # signature doesn't do anything with it. If that changes (if
        # .buildFinished's return value becomes significant), we need to
        # rearrange this.
        return self.buildMessage(name, build, results)

    def messageFormatter(self, mode, name, build, results, master_status):
        """Customized email message"""
        result = Results[results]

        source = ""
        ss = build.getSourceStamp()
        if ss:
            if ss.branch:
                source += "[branch %s] " % ss.branch
            if ss.revision:
                source +=  ss.revision
            else:
                source += "HEAD"
            if ss.patch:
                source += " (plus patch)"

        logs = list()
        for log in build.getLogs():
            log_name = "%s.%s" % (log.getStep().getName(), log.getName())
            log_status, dummy = log.getStep().getResults()
            log_body = log.getText().splitlines() # Note: can be VERY LARGE
            log_url = '%s/steps/%s/logs/%s' % (master_status.getURLForThing(build),
                                               log.getStep().getName(),
                                               log.getName())
            logs.append((log_name, log_url, log_body, log_status))
     
        logname, logurl, logcontent, logstatus = logs[-1]
     
        subject = "Buildbot: %s %s %s" % (logname, result.upper(), source)

        text = list()
        text.append('<h4>%s %s</h4>' % (logname, result.upper()))
        text.append("Buildslave for this Build: <b>%s</b> (but I might be suppressing others)" % build.getSlavename())
        text.append('<br>')
        if master_status.getURLForThing(build):
            text.append('Logs: <a href="%s">%s</a>'
                        % (master_status.getURLForThing(build),
                           master_status.getURLForThing(build))
                        )
        text.append('<br>')
        text.append("Build Reason: %s" % build.getReason())
        text.append('<br>')

        text.append("Build Source Stamp: <b>%s</b>" % (source,))
        text.append('<br>')
        text.append("Blamelist: %s" % ",".join(build.getResponsibleUsers()))
        text.append('<br>')

        if ss and ss.changes:
            text.append('<h4>Changes:</h4>')
            text.extend([c.asHTML() for c in ss.changes])
     
        text.append('<i>Detailed log of last build step:</i> <a href="%s">%s</a>'
                    % (logurl, logurl))
        text.append('<br>')
        if name == 'roottest' and len(logcontent) and not 'too many' in logcontent[0]:
            text.append('<ul>')
            for l in logcontent:
                text.append('<li>%s</li>' % (l))
            text.append('</ul>')
        text.append('<br><br>')
        text.append('<b>-The BuildBot</b>')

        return {'type': 'html',
               'body': "\n".join(text),
               'subject' : subject}

class ROOTBuildBotConfig:
    """Collect configuration options and spread them into buildbot
    in a way consistent for ROOT's build setup."""

    def __init__(self):
        self.c = { 'slaves' : [],
                   'status': [],
                   'change_source': [],
                   'schedulers' : [],
                   'builders': [] }
        self.slaves = {}
        self.sources = {}
        self.builders = set()
        self.svnurl = 'http://root.cern.ch/svn/root/trunk'
        self.locks = {}

    def addSlave(self, name, arch, envScript = None, configFlags = [], max_builds = 2, vetoSource = []):
        if type(configFlags) is str: configFlags = [ configFlags ]
        if type(vetoSource) is str : vetoSource  = [ vetoSource ]
        # each architecture is built on exactly one slave:
        self.slaves[arch] = { 'name' : name,
                              'configFlags' : configFlags,
                              'envScript' : envScript,
                              'max_builds' : max_builds,
                              'vetoSource' : vetoSource }

    def addSource(self, name, make, svnurl, svncheckurl, viewvc, schedule, triggeredby = None,
                  configure = "./configure", configFlags = [], envScript = None):
        if type(configFlags) is str: configFlags = [ configFlags ]
        self.sources[name] = ROOTBuildSource(name, make, svnurl, svncheckurl,
                                             viewvc, triggeredby,
                                             configure, configFlags,
                                             envScript, schedule)
        if (triggeredby != None):
            self.sources[triggeredby].addTrigger(name)

    def addWebStatus(self, httpPort, allowForce):
        stat = html.WebStatus(http_port = httpPort,
                              allowForce = allowForce)
        self.c['status'].append(stat)

    def configureBuildmaster(self):
        self.c['projectName'] = "ROOT"
        self.c['projectURL'] = "http://root.cern.ch/"
        #self.c['buildbotURL'] = "http://lxroot01.cern.ch:8010/" # FIXME PCROOT
        self.c['mergeRequests'] = self.mergeRequests

        self.configureSlaves()
        self.configureSource()
        self.configureSchedulers()
        self.sortBuilders()
        self.configureNotifications()

        return self.c

    def configureSlaves(self):
        self.c['slavePortnum'] = 9360
        password = file('passwd_buildbot', 'rb').readlines()[0].strip()

        for arch, slave in self.slaves.iteritems():
            self.c['slaves'].append(BuildSlave(slave['name'],
                                               password,
                                               slave['max_builds']))

    def configureSource(self):
        self.c['changeHorizon'] = 1000 # up to 1000 changes
        for srcname, src in self.sources.iteritems():
            self.c['change_source'].append(
                SVNPoller(
                    svnurl=src.svnurl, 
                    pollinterval=10, # seconds
                    histmax=10,
                    svnbin='svn',
                    revlinktmpl=src.viewvc,
                    category=src.name
                    )
                )

    def configureSchedulers(self):
        scheds = {'incr'     : self.makeIncrScheduler,
                  'full'     : self.makeFullScheduler,
                  'hourly'   : self.makeHourlyScheduler,
                  'try'      : self.makeTryScheduler,
                  'triggered': self.makeTriggerScheduler
                  }

        for srcname, src in self.sources.iteritems():
            schedule = src.schedule
            if src.triggeredby != None: schedule += ':triggered'
            splitsched = schedule.split(':')
            for sched in splitsched:
                schedBuilders = []
                maker = scheds[sched]
                for arch, slave in self.slaves.iteritems():
                    if srcname in slave['vetoSource']: continue
                    if sched == 'triggered':
                        depsched = self.sources[src.triggeredby].schedule
                        for depsched in depsched.split(':'):
                            bldname = self.generateBuilder(src, slave, arch, depsched)
                            schedBuilders.append(bldname)
                    else :
                        bldname = self.generateBuilder(src, slave, arch, sched)
                        schedBuilders.append(bldname)
                maker(src, schedBuilders)

    def makeIncrScheduler(self, src, builders):
        name = src.name + '-incr'
        prev = filter(lambda s: s.name == name, self.c['schedulers'])
        if len(prev) : prev[0].builderNames.extend(builders)
        else :
            sched = Scheduler(name = name,
                              branch = None,
                              treeStableTimer = 0,
                              fileIsImportant = src.fileIsImportant,
                              builderNames = builders,
                              categories = (src.name))
            self.c['schedulers'].append(sched)
        
    def makeFullScheduler(self, src, builders):
        name = src.name + '-full'
        prev = filter(lambda s: s.name == name, self.c['schedulers'])
        if len(prev) : prev[0].builderNames.extend(builders)
        else :
            sched = Scheduler(name = name,
                              branch = None,
                              treeStableTimer = 0,
                              fileIsImportant = src.fileIsImportant,
                              builderNames = builders,
                              categories = ('Only triggered on explicit request!'))
            self.c['schedulers'].append(sched)
        
    def makeHourlyScheduler(self, src, builders):
        name = src.name + '-hourly'
        prev = filter(lambda s: s.name == name, self.c['schedulers'])
        if len(prev) : prev[0].builderNames.extend(builders)
        else :
            sched = scheduler.Nightly(name = name,
                                      builderNames = builders,
                                      #hour = at each hour
                                      minute = 0,
                                      onlyIfChanged = True)
            self.c['schedulers'].append(sched)

    def makeTryScheduler(self, src, builders):
        name = src.name + '-try'
        prev = filter(lambda s: s.name == name, self.c['schedulers'])
        if len(prev) : prev[0].builderNames.extend(builders)
        else :
            sched = Try_Jobdir(name = name,
                               builderNames = builders,
                               jobdir = 'try')
            self.c['schedulers'].append(sched)

    def makeTriggerScheduler(self, src, builders):
        # need one trigger per arch:
        #  once GCC4.4 on UbuntuXY is done, THAT
        #  roottest needs to be started
        for bld in builders:
            # name is SRC-ARCH-incr-triggerable
            name = bld + '-triggerable'
            prev = filter(lambda s: s.name == name, self.c['schedulers'])
            if len(prev) : prev[0].builderNames.append(bld)
            else :
                sched = scheduler.Triggerable(name = name,
                                              builderNames = [ bld ])
                self.c['schedulers'].append(sched)

    def generateBuilder(self, src, slave, arch, buildertype):
        name = src.name + '-' + arch + '-' + buildertype
        if name in self.builders: return name

        mode = 'copy'
        if buildertype == 'incr' and src.name == 'ROOT': mode = 'update'

        # if we are triggered then we need to use the latest revision:
        alwaysUseLatest = False
        if src.triggeredby != None: alwaysUseLatest = True

        confLine = "bash -c "
        compLine = "bash -c "
        haveExtraArgs = False
        if ('envScript' in slave and slave['envScript']) \
                or src.envScript:
            confLine += '"'
            compLine += '"'
            haveExtraArgs = True
        if 'envScript' in slave and slave['envScript']:
            confLine += slave['envScript'] + " && "
            compLine += slave['envScript'] + " && "
        if src.envScript:
            confLine += src.envScript + " && "
            compLine += src.envScript + " && "
            
        confLine += str(src.configure)
        compLine += str(src.make)

        confArgs = ""
        for flag in slave['configFlags']:
            confArgs += ' ' + flag
        for flag in src.configFlags:
            confArgs += '  ' + flag
        confLine += confArgs
        # make script wants config args in case configure needs to be called
        if src.name == 'ROOT': compLine += confArgs

        if haveExtraArgs:
            confLine += '"'
            compLine += '"'

        if src.configure == None or buildertype == 'incr':
            confLine = None

        fact = None
        svn = ROOTSVN(mode = mode,
                      svnurl = src.svnurl,
                      alwaysUseLatest = alwaysUseLatest,
                      retry = (20, 6),
                      category = src.name)
        if src.name == 'ROOT':
            fact = factory.GNUAutoconf(svn,
                                       configure = confLine,
                                       configureFlags = [],
                                       compile = compLine,
                                       test = None)
        else:
            fact = factory.BuildFactory()
            fact.addStep(svn)
            fact.addStep(ROOTTestCmd(command = compLine))

        if buildertype == 'incr' and src.name != 'roottest':
            fact.useProgress = False

        if len(src.triggers):
            trigsched = []
            for trig in src.triggers:
                # the scheduler name is like
                #   roottest-ARCH-full-triggerable
                trigsched.append(trig + '-' + arch + '-' + buildertype + '-triggerable')
            fact.addStep(trigger.Trigger(schedulerNames = trigsched,
                                         # it's bad to rebuild ROOT while its roottest is using it,
                                         # but waiting to finish can end up in a deadlock
                                         # where the node cannot build roottest because it's busy with a ROOT
                                         # build waiting for roottest to finish...
                                         # Instead, use locks on the builders
                                         waitForFinish = False,
                                         # DO NOT use src's SVN revision but
                                         # the one from the trigger's src:
                                         updateSourceStamp = False))

        buildtypesort = {
            'incr'     : 0,
            'triggered': 1,
            'hourly'   : 2,
            'full'     : 90,
            'try'      : 99
            }

        srcnamesort = src.name.split('-')[0].upper()

        # ROOT and roottest cannot run at the same time on the same builder
        lockname = arch + '-' + slave['name'] + '-' + buildertype
        if lockname in self.locks:
            build_lock = self.locks[lockname]
        else:
            build_lock = locks.SlaveLock(lockname)
            self.locks[lockname] = build_lock

        bld = {
            'name'     : name,
            'slavename': slave['name'],
            'builddir' : name,
            'factory'  : fact,
            'category' : src.name + '-' + buildertype,
            'locks'    : [build_lock.access('exclusive')],
            'sort'     : (buildtypesort[buildertype], arch, srcnamesort) # defines sort order
            }

        self.builders.add(name)
        self.c['builders'].append(bld)

        return name

    def sortBuilders(self):
        self.c['builders'] = sorted(self.c['builders'], key=lambda x: (x['sort'][0],x['sort'][1],x['sort'][2]))

    def mergeRequests(self, builder, req1, req2):
        if req1.source.canBeMergedWith(req2.source): return True
        if req1.properties['owner'] == req2.properties['owner']: return True # we can blame the same submitter
        if req1.source.revision is None or req2.source.revision is None: return False
        # Last chance: if change log references other revision it probably fixes it; merge them:
        if req1.reason.find(str(req2.source.revision)) == -1 and req2.reason.find(str(req1.source.revision)) == -1: return False
        return True

    def configureNotifications(self):
        mn = ROOTMailNotifier()
        self.c['status'].append(mn)


