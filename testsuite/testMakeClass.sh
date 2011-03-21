#
# script to create a standalone executable 
# to test all TMVA standalone-classe
# ToDo: make it also work on other shells as batch
#
# E. v. Toerne
# 
fil=testMakeClass.cxx
echo " " > $fil
for i in `ls weights/testmakeclass_*.C`; do
echo "#include \"$i\"" >> $fil
done
echo "int main(){" >> $fil
for i in `ls weights/testmakeclass_*.C`; do
    meth=`echo $i | sed "s/weights\///" | sed "s/\.C//"`
    echo "$meth();" >> $fil
done
echo "}" >> $fil
