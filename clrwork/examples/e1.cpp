//Generated at Wed Jun  9 18:25:12 2010
//Do not modify.

#include "Reflex/Builder/ReflexBuilder.h"
#include <typeinfo>
using namespace Reflex;

namespace {
}

// ---------------------- Shadow classes -----------------------------------
namespace __shadow__ {

}

// ---------------------- Stub functions -----------------------------------
namespace {

static void* function_0(void*, const std::vector<void*>& arg, void*)
{
   return (void*)operator new(*(unsigned int*) arg[0]);
}

static void* function_1(void*, const std::vector<void*>& arg, void*)
{
   return (void*)operator new[](*(unsigned int*) arg[0]);
}

static void* function_2(void*, const std::vector<void*>& arg, void*)
{
   operator delete((void*) arg[0]);
   return 0;
}

static void* function_3(void*, const std::vector<void*>& arg, void*)
{
   operator delete[]((void*) arg[0]);
   return 0;
}

// -- Stub functions for class TGSplitButton--
static void* constructor_4(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGSplitButton(*(const TGSplitButton*) arg[0]);
}

static void* method_5(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGSplitButton*)o)->operator=(*(const TGSplitButton*) arg[0]);
}

static void* method_6(void* o, const std::vector<void*>&, void*)
{
   ((TGSplitButton*)o)->CalcSize();
   return 0;
}

static void* method_7(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->DrawTriangle(*(const GContext_t*) arg[0], *(Int_t*) arg[1], *(Int_t*) arg[2]);
   return 0;
}

static void* method_8(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGSplitButton*)o)->HandleSButton((Event_t*) arg[0]));
}

static void* method_9(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGSplitButton*)o)->HandleSCrossing((Event_t*) arg[0]));
}

static void* method_10(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGSplitButton*)o)->HandleSKey((Event_t*) arg[0]));
}

static void* method_11(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->SetMenuState(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_12(void* o, const std::vector<void*>&, void*)
{
   ((TGSplitButton*)o)->DoRedraw();
   return 0;
}

static void* method_13(void* o, const std::vector<void*>&, void*)
{
   ((TGSplitButton*)o)->Init();
   return 0;
}

static void* method_14(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->BindKeys(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_15(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->BindMenuKeys(*(Bool_t*) arg[0]);
   return 0;
}
static void* constructor_16(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGSplitButton((const TGWindow*) arg[0], (TGHotString*) arg[1], (TGPopupMenu*) arg[2], *(Bool_t*) arg[3], *(Int_t*) arg[4], *(GContext_t*) arg[5], *(FontStruct_t*) arg[6], *(UInt_t*) arg[7]);
}
static void* destructor_17(void * o, const std::vector<void*>&, void *)
{
   ((TGSplitButton*)o)->~TGSplitButton();
   return 0;
}

static void* method_18(void* o, const std::vector<void*>&, void*)
{
   return new TGDimension(((TGSplitButton*)o)->GetDefaultSize());
}

static void* method_19(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->SetText((TGHotString*) arg[0]);
   return 0;
}

static void* method_20(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->SetText(*(const TString*) arg[0]);
   return 0;
}

static void* method_21(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->SetFont(*(FontStruct_t*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_22(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->SetFont((const char*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_23(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->SetMBState(*(EButtonState*) arg[0]);
   return 0;
}

static void* method_24(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->SetSplit(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_25(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGSplitButton*)o)->IsSplit());
}

static void* method_26(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGSplitButton*)o)->HandleButton((Event_t*) arg[0]));
}

static void* method_27(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGSplitButton*)o)->HandleCrossing((Event_t*) arg[0]));
}

static void* method_28(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGSplitButton*)o)->HandleKey((Event_t*) arg[0]));
}

static void* method_29(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGSplitButton*)o)->HandleMotion((Event_t*) arg[0]));
}

static void* method_30(void* o, const std::vector<void*>&, void*)
{
   ((TGSplitButton*)o)->Layout();
   return 0;
}

static void* method_31(void* o, const std::vector<void*>&, void*)
{
   ((TGSplitButton*)o)->MBPressed();
   return 0;
}

static void* method_32(void* o, const std::vector<void*>&, void*)
{
   ((TGSplitButton*)o)->MBReleased();
   return 0;
}

static void* method_33(void* o, const std::vector<void*>&, void*)
{
   ((TGSplitButton*)o)->MBClicked();
   return 0;
}

static void* method_34(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->ItemClicked(*(Int_t*) arg[0]);
   return 0;
}

static void* method_35(void* o, const std::vector<void*>& arg, void*)
{
   ((TGSplitButton*)o)->HandleMenu(*(Int_t*) arg[0]);
   return 0;
}

// -- Stub functions for class TGRadioButton--
static void* constructor_36(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGRadioButton(*(const TGRadioButton*) arg[0]);
}

static void* method_37(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGRadioButton*)o)->operator=(*(const TGRadioButton*) arg[0]);
}

static void* method_38(void* o, const std::vector<void*>&, void*)
{
   ((TGRadioButton*)o)->Init();
   return 0;
}

static void* method_39(void* o, const std::vector<void*>& arg, void*)
{
   ((TGRadioButton*)o)->PSetState(*(EButtonState*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_40(void* o, const std::vector<void*>&, void*)
{
   ((TGRadioButton*)o)->DoRedraw();
   return 0;
}

static void* method_41(void* o, const std::vector<void*>& arg, void*)
{
   ((TGRadioButton*)o)->EmitSignals(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_42(void* o, const std::vector<void*>&, void*)
{
   return new FontStruct_t(((TGRadioButton*)o)->GetDefaultFontStruct());
}

static void* method_43(void* o, const std::vector<void*>&, void*)
{
   return (void*) & ((TGRadioButton*)o)->GetDefaultGC();
}
static void* constructor_44(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGRadioButton((const TGWindow*) arg[0], (TGHotString*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(FontStruct_t*) arg[4], *(UInt_t*) arg[5]);
}
static void* constructor_45(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGRadioButton((const TGWindow*) arg[0], (const char*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(FontStruct_t*) arg[4], *(UInt_t*) arg[5]);
}
static void* constructor_46(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGRadioButton((const TGWindow*) arg[0], (const char*) arg[1], (const char*) arg[2], *(Int_t*) arg[3], *(GContext_t*) arg[4], *(FontStruct_t*) arg[5], *(UInt_t*) arg[6]);
}
static void* destructor_47(void * o, const std::vector<void*>&, void *)
{
   ((TGRadioButton*)o)->~TGRadioButton();
   return 0;
}

static void* method_48(void* o, const std::vector<void*>&, void*)
{
   return new TGDimension(((TGRadioButton*)o)->GetDefaultSize());
}

static void* method_49(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGRadioButton*)o)->HandleButton((Event_t*) arg[0]));
}

static void* method_50(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGRadioButton*)o)->HandleKey((Event_t*) arg[0]));
}

static void* method_51(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGRadioButton*)o)->HandleCrossing((Event_t*) arg[0]));
}

static void* method_52(void* o, const std::vector<void*>& arg, void*)
{
   ((TGRadioButton*)o)->SetState(*(EButtonState*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_53(void* o, const std::vector<void*>& arg, void*)
{
   ((TGRadioButton*)o)->SetDisabledAndSelected(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_54(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGRadioButton*)o)->IsToggleButton());
}

static void* method_55(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGRadioButton*)o)->IsExclusiveToggle());
}

static void* method_56(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGRadioButton*)o)->IsOn());
}

static void* method_57(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGRadioButton*)o)->IsDown());
}

static void* method_58(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGRadioButton*)o)->IsDisabledAndSelected());
}

static void* method_59(void* o, const std::vector<void*>& arg, void*)
{
   ((TGRadioButton*)o)->SavePrimitive(*(ostream*) arg[0], (const Option_t*) arg[1]);
   return 0;
}

// -- Stub functions for class TGCheckButton--
static void* constructor_60(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGCheckButton(*(const TGCheckButton*) arg[0]);
}

static void* method_61(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGCheckButton*)o)->operator=(*(const TGCheckButton*) arg[0]);
}

static void* method_62(void* o, const std::vector<void*>&, void*)
{
   ((TGCheckButton*)o)->Init();
   return 0;
}

static void* method_63(void* o, const std::vector<void*>& arg, void*)
{
   ((TGCheckButton*)o)->PSetState(*(EButtonState*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_64(void* o, const std::vector<void*>&, void*)
{
   ((TGCheckButton*)o)->DoRedraw();
   return 0;
}

static void* method_65(void* o, const std::vector<void*>& arg, void*)
{
   ((TGCheckButton*)o)->EmitSignals(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_66(void* o, const std::vector<void*>&, void*)
{
   return new FontStruct_t(((TGCheckButton*)o)->GetDefaultFontStruct());
}

static void* method_67(void* o, const std::vector<void*>&, void*)
{
   return (void*) & ((TGCheckButton*)o)->GetDefaultGC();
}
static void* constructor_68(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGCheckButton((const TGWindow*) arg[0], (TGHotString*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(FontStruct_t*) arg[4], *(UInt_t*) arg[5]);
}
static void* constructor_69(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGCheckButton((const TGWindow*) arg[0], (const char*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(FontStruct_t*) arg[4], *(UInt_t*) arg[5]);
}
static void* constructor_70(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGCheckButton((const TGWindow*) arg[0], (const char*) arg[1], (const char*) arg[2], *(Int_t*) arg[3], *(GContext_t*) arg[4], *(FontStruct_t*) arg[5], *(UInt_t*) arg[6]);
}
static void* destructor_71(void * o, const std::vector<void*>&, void *)
{
   ((TGCheckButton*)o)->~TGCheckButton();
   return 0;
}

static void* method_72(void* o, const std::vector<void*>&, void*)
{
   return new TGDimension(((TGCheckButton*)o)->GetDefaultSize());
}

static void* method_73(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGCheckButton*)o)->HandleButton((Event_t*) arg[0]));
}

static void* method_74(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGCheckButton*)o)->HandleKey((Event_t*) arg[0]));
}

static void* method_75(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGCheckButton*)o)->HandleCrossing((Event_t*) arg[0]));
}

static void* method_76(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGCheckButton*)o)->IsToggleButton());
}

static void* method_77(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGCheckButton*)o)->IsOn());
}

static void* method_78(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGCheckButton*)o)->IsDown());
}

static void* method_79(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGCheckButton*)o)->IsDisabledAndSelected());
}

static void* method_80(void* o, const std::vector<void*>& arg, void*)
{
   ((TGCheckButton*)o)->SetDisabledAndSelected(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_81(void* o, const std::vector<void*>& arg, void*)
{
   ((TGCheckButton*)o)->SetState(*(EButtonState*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_82(void* o, const std::vector<void*>& arg, void*)
{
   ((TGCheckButton*)o)->SavePrimitive(*(ostream*) arg[0], (const Option_t*) arg[1]);
   return 0;
}

// -- Stub functions for class TGPictureButton--

static void* method_83(void* o, const std::vector<void*>&, void*)
{
   ((TGPictureButton*)o)->DoRedraw();
   return 0;
}

static void* method_84(void* o, const std::vector<void*>&, void*)
{
   ((TGPictureButton*)o)->CreateDisabledPicture();
   return 0;
}
static void* constructor_85(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGPictureButton(*(const TGPictureButton*) arg[0]);
}

static void* method_86(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGPictureButton*)o)->operator=(*(const TGPictureButton*) arg[0]);
}
static void* constructor_87(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGPictureButton((const TGWindow*) arg[0], (const TGPicture*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(UInt_t*) arg[4]);
}
static void* constructor_88(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGPictureButton((const TGWindow*) arg[0], (const TGPicture*) arg[1], (const char*) arg[2], *(Int_t*) arg[3], *(GContext_t*) arg[4], *(UInt_t*) arg[5]);
}
static void* constructor_89(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGPictureButton((const TGWindow*) arg[0], (const char*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(UInt_t*) arg[4]);
}
static void* destructor_90(void * o, const std::vector<void*>&, void *)
{
   ((TGPictureButton*)o)->~TGPictureButton();
   return 0;
}

static void* method_91(void* o, const std::vector<void*>& arg, void*)
{
   ((TGPictureButton*)o)->SetPicture((const TGPicture*) arg[0]);
   return 0;
}

static void* method_92(void* o, const std::vector<void*>& arg, void*)
{
   ((TGPictureButton*)o)->SetDisabledPicture((const TGPicture*) arg[0]);
   return 0;
}

static void* method_93(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGPictureButton*)o)->GetPicture();
}

static void* method_94(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGPictureButton*)o)->GetDisabledPicture();
}

static void* method_95(void* o, const std::vector<void*>& arg, void*)
{
   ((TGPictureButton*)o)->SavePrimitive(*(ostream*) arg[0], (const Option_t*) arg[1]);
   return 0;
}

// -- Stub functions for class TGTextButton--

static void* method_96(void* o, const std::vector<void*>&, void*)
{
   ((TGTextButton*)o)->Init();
   return 0;
}

static void* method_97(void* o, const std::vector<void*>&, void*)
{
   ((TGTextButton*)o)->DoRedraw();
   return 0;
}
static void* constructor_98(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGTextButton(*(const TGTextButton*) arg[0]);
}

static void* method_99(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGTextButton*)o)->operator=(*(const TGTextButton*) arg[0]);
}

static void* method_100(void* o, const std::vector<void*>&, void*)
{
   return new FontStruct_t(((TGTextButton*)o)->GetDefaultFontStruct());
}
static void* constructor_101(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGTextButton((const TGWindow*) arg[0], (TGHotString*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(FontStruct_t*) arg[4], *(UInt_t*) arg[5]);
}
static void* constructor_102(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGTextButton((const TGWindow*) arg[0], (const char*) arg[1], *(Int_t*) arg[2], *(GContext_t*) arg[3], *(FontStruct_t*) arg[4], *(UInt_t*) arg[5]);
}
static void* constructor_103(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGTextButton((const TGWindow*) arg[0], (const char*) arg[1], (const char*) arg[2], *(Int_t*) arg[3], *(GContext_t*) arg[4], *(FontStruct_t*) arg[5], *(UInt_t*) arg[6]);
}
static void* destructor_104(void * o, const std::vector<void*>&, void *)
{
   ((TGTextButton*)o)->~TGTextButton();
   return 0;
}

static void* method_105(void* o, const std::vector<void*>&, void*)
{
   return new TGDimension(((TGTextButton*)o)->GetDefaultSize());
}

static void* method_106(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGTextButton*)o)->HandleKey((Event_t*) arg[0]));
}

static void* method_107(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGTextButton*)o)->GetText();
}

static void* method_108(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGTextButton*)o)->GetTitle();
}

static void* method_109(void* o, const std::vector<void*>&, void*)
{
   return new TString(((TGTextButton*)o)->GetString());
}

static void* method_110(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetTextJustify(*(Int_t*) arg[0]);
   return 0;
}

static void* method_111(void* o, const std::vector<void*>&, void*)
{
   return new Int_t(((TGTextButton*)o)->GetTextJustify());
}

static void* method_112(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetText((TGHotString*) arg[0]);
   return 0;
}

static void* method_113(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetText(*(const TString*) arg[0]);
   return 0;
}

static void* method_114(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetTitle((const char*) arg[0]);
   return 0;
}

static void* method_115(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetFont(*(FontStruct_t*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_116(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetFont((const char*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_117(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetTextColor(*(Pixel_t*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_118(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetForegroundColor(*(Pixel_t*) arg[0]);
   return 0;
}

static void* method_119(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGTextButton*)o)->HasOwnFont());
}

static void* method_120(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetWrapLength(*(Int_t*) arg[0]);
   return 0;
}

static void* method_121(void* o, const std::vector<void*>&, void*)
{
   return new Int_t(((TGTextButton*)o)->GetWrapLength());
}

static void* method_122(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetMargins(*(Int_t*) arg[0], *(Int_t*) arg[1], *(Int_t*) arg[2], *(Int_t*) arg[3]);
   return 0;
}

static void* method_123(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetLeftMargin(*(Int_t*) arg[0]);
   return 0;
}

static void* method_124(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetRightMargin(*(Int_t*) arg[0]);
   return 0;
}

static void* method_125(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetTopMargin(*(Int_t*) arg[0]);
   return 0;
}

static void* method_126(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SetBottomMargin(*(Int_t*) arg[0]);
   return 0;
}

static void* method_127(void* o, const std::vector<void*>&, void*)
{
   return new Int_t(((TGTextButton*)o)->GetLeftMargin());
}

static void* method_128(void* o, const std::vector<void*>&, void*)
{
   return new Int_t(((TGTextButton*)o)->GetRightMargin());
}

static void* method_129(void* o, const std::vector<void*>&, void*)
{
   return new Int_t(((TGTextButton*)o)->GetTopMargin());
}

static void* method_130(void* o, const std::vector<void*>&, void*)
{
   return new Int_t(((TGTextButton*)o)->GetBottomMargin());
}

static void* method_131(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->ChangeText((const char*) arg[0]);
   return 0;
}

static void* method_132(void* o, const std::vector<void*>&, void*)
{
   return new FontStruct_t(((TGTextButton*)o)->GetFontStruct());
}

static void* method_133(void* o, const std::vector<void*>&, void*)
{
   ((TGTextButton*)o)->Layout();
   return 0;
}

static void* method_134(void* o, const std::vector<void*>& arg, void*)
{
   ((TGTextButton*)o)->SavePrimitive(*(ostream*) arg[0], (const Option_t*) arg[1]);
   return 0;
}

// -- Stub functions for class TGButton--

static void* method_135(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetToggleButton(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_136(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->EmitSignals(*(Bool_t*) arg[0]);
   return 0;
}
static void* constructor_137(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGButton(*(const TGButton*) arg[0]);
}

static void* method_138(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGButton*)o)->operator=(*(const TGButton*) arg[0]);
}

static void* method_139(void* o, const std::vector<void*>&, void*)
{
   return (void*) & ((TGButton*)o)->GetDefaultGC();
}

static void* method_140(void* o, const std::vector<void*>&, void*)
{
   return (void*) & ((TGButton*)o)->GetHibckgndGC();
}
static void* constructor_141(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGButton((const TGWindow*) arg[0], *(Int_t*) arg[1], *(GContext_t*) arg[2], *(UInt_t*) arg[3]);
}
static void* destructor_142(void * o, const std::vector<void*>&, void *)
{
   ((TGButton*)o)->~TGButton();
   return 0;
}

static void* method_143(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGButton*)o)->HandleButton((Event_t*) arg[0]));
}

static void* method_144(void* o, const std::vector<void*>& arg, void*)
{
   return new Bool_t(((TGButton*)o)->HandleCrossing((Event_t*) arg[0]));
}

static void* method_145(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetUserData((void*) arg[0]);
   return 0;
}

static void* method_146(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGButton*)o)->GetUserData();
}

static void* method_147(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetToolTipText((const char*) arg[0], *(Long_t*) arg[1]);
   return 0;
}

static void* method_148(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGButton*)o)->GetToolTip();
}

static void* method_149(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetState(*(EButtonState*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_150(void* o, const std::vector<void*>&, void*)
{
   return new EButtonState(((TGButton*)o)->GetState());
}

static void* method_151(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->AllowStayDown(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_152(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetGroup((TGButtonGroup*) arg[0]);
   return 0;
}

static void* method_153(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGButton*)o)->GetGroup();
}

static void* method_154(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGButton*)o)->IsDown());
}

static void* method_155(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetDown(*(Bool_t*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_156(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGButton*)o)->IsOn());
}

static void* method_157(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetOn(*(Bool_t*) arg[0], *(Bool_t*) arg[1]);
   return 0;
}

static void* method_158(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGButton*)o)->IsToggleButton());
}

static void* method_159(void* o, const std::vector<void*>&, void*)
{
   return new Bool_t(((TGButton*)o)->IsExclusiveToggle());
}

static void* method_160(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->Toggle(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_161(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SetEnabled(*(Bool_t*) arg[0]);
   return 0;
}

static void* method_162(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->SavePrimitive(*(ostream*) arg[0], (const Option_t*) arg[1]);
   return 0;
}

static void* method_163(void* o, const std::vector<void*>&, void*)
{
   return new GContext_t(((TGButton*)o)->GetNormGC());
}

static void* method_164(void* o, const std::vector<void*>&, void*)
{
   ((TGButton*)o)->Pressed();
   return 0;
}

static void* method_165(void* o, const std::vector<void*>&, void*)
{
   ((TGButton*)o)->Released();
   return 0;
}

static void* method_166(void* o, const std::vector<void*>&, void*)
{
   ((TGButton*)o)->Clicked();
   return 0;
}

static void* method_167(void* o, const std::vector<void*>& arg, void*)
{
   ((TGButton*)o)->Toggled(*(Bool_t*) arg[0]);
   return 0;
}

// -- Stub functions for class TGTextLayout--

// -- Stub functions for class TGPopupMenu--

// -- Stub functions for class TGToolTip--

// -- Stub functions for class TGPicture--

// -- Stub functions for class ostream--
static void* constructor_168(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) ostream();
}
static void* constructor_169(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) ostream(*(const ostream*) arg[0]);
}

static void* method_170(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((ostream*)o)->operator=(*(const ostream*) arg[0]);
}
static void* destructor_171(void * o, const std::vector<void*>&, void *)
{
   ((ostream*)o)->~ostream();
   return 0;
}

// -- Stub functions for class TGButtonGroup--
static void* constructor_172(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) TGButtonGroup();
}
static void* constructor_173(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGButtonGroup(*(const TGButtonGroup*) arg[0]);
}

static void* method_174(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGButtonGroup*)o)->operator=(*(const TGButtonGroup*) arg[0]);
}
static void* destructor_175(void * o, const std::vector<void*>&, void *)
{
   ((TGButtonGroup*)o)->~TGButtonGroup();
   return 0;
}

// -- Stub functions for class TGFrame--
static void* constructor_176(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) TGFrame();
}
static void* constructor_177(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGFrame(*(const TGFrame*) arg[0]);
}

static void* method_178(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGFrame*)o)->operator=(*(const TGFrame*) arg[0]);
}
static void* destructor_179(void * o, const std::vector<void*>&, void *)
{
   ((TGFrame*)o)->~TGFrame();
   return 0;
}

// -- Stub functions for class TGWindow--
static void* constructor_180(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) TGWindow();
}
static void* constructor_181(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGWindow(*(const TGWindow*) arg[0]);
}

static void* method_182(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGWindow*)o)->operator=(*(const TGWindow*) arg[0]);
}
static void* destructor_183(void * o, const std::vector<void*>&, void *)
{
   ((TGWindow*)o)->~TGWindow();
   return 0;
}

// -- Stub functions for class TGWidget--

static void* method_184(void* o, const std::vector<void*>& arg, void*)
{
   ((TGWidget*)o)->Emit((const char*) arg[0], *(_Bool*) arg[1]);
   return 0;
}
static void* constructor_185(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) TGWidget();
}
static void* constructor_186(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGWidget(*(const TGWidget*) arg[0]);
}

static void* method_187(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGWidget*)o)->operator=(*(const TGWidget*) arg[0]);
}
static void* destructor_188(void * o, const std::vector<void*>&, void *)
{
   ((TGWidget*)o)->~TGWidget();
   return 0;
}

// -- Stub functions for class TGGC--

static void* method_189(void* o, const std::vector<void*>&, void*)
{
   return new GContext_t(((TGGC*)o)->operator()());
}
static void* constructor_190(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) TGGC();
}
static void* constructor_191(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGGC(*(const TGGC*) arg[0]);
}

static void* method_192(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGGC*)o)->operator=(*(const TGGC*) arg[0]);
}
static void* destructor_193(void * o, const std::vector<void*>&, void *)
{
   ((TGGC*)o)->~TGGC();
   return 0;
}

// -- Stub functions for class TGHotString--
static void* constructor_194(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGHotString((const char*) arg[0]);
}
static void* constructor_195(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGHotString(*(const TGHotString*) arg[0]);
}

static void* method_196(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGHotString*)o)->operator=(*(const TGHotString*) arg[0]);
}
static void* destructor_197(void * o, const std::vector<void*>&, void *)
{
   ((TGHotString*)o)->~TGHotString();
   return 0;
}

// -- Stub functions for class TGString--
static void* constructor_198(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGString((const char*) arg[0]);
}

static void* method_199(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGString*)o)->Data();
}

static void* method_200(void* o, const std::vector<void*>&, void*)
{
   return (void*)((TGString*)o)->GetString();
}
static void* constructor_201(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGString(*(const TGString*) arg[0]);
}

static void* method_202(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGString*)o)->operator=(*(const TGString*) arg[0]);
}
static void* destructor_203(void * o, const std::vector<void*>&, void *)
{
   ((TGString*)o)->~TGString();
   return 0;
}

// -- Stub functions for class TString--
static void* constructor_204(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TString((const char*) arg[0]);
}
static void* constructor_205(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TString(*(const TString*) arg[0]);
}

static void* method_206(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TString*)o)->operator=(*(const TString*) arg[0]);
}
static void* destructor_207(void * o, const std::vector<void*>&, void *)
{
   ((TString*)o)->~TString();
   return 0;
}

// -- Stub functions for class TGDimension--
static void* constructor_208(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) TGDimension();
}
static void* constructor_209(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGDimension(*(const TGDimension*) arg[0]);
}

static void* method_210(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGDimension*)o)->operator=(*(const TGDimension*) arg[0]);
}
static void* destructor_211(void * o, const std::vector<void*>&, void *)
{
   ((TGDimension*)o)->~TGDimension();
   return 0;
}

// -- Stub functions for class TGFont--
static void* constructor_212(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) TGFont();
}
static void* constructor_213(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) TGFont(*(const TGFont*) arg[0]);
}

static void* method_214(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((TGFont*)o)->operator=(*(const TGFont*) arg[0]);
}
static void* destructor_215(void * o, const std::vector<void*>&, void *)
{
   ((TGFont*)o)->~TGFont();
   return 0;
}

// -- Stub functions for class Cursor_t--
static void* constructor_216(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) Cursor_t();
}
static void* constructor_217(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) Cursor_t(*(const Cursor_t*) arg[0]);
}

static void* method_218(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((Cursor_t*)o)->operator=(*(const Cursor_t*) arg[0]);
}
static void* destructor_219(void * o, const std::vector<void*>&, void *)
{
   ((Cursor_t*)o)->~Cursor_t();
   return 0;
}

// -- Stub functions for class Pixel_t--
static void* constructor_220(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) Pixel_t();
}
static void* constructor_221(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) Pixel_t(*(const Pixel_t*) arg[0]);
}

static void* method_222(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((Pixel_t*)o)->operator=(*(const Pixel_t*) arg[0]);
}
static void* destructor_223(void * o, const std::vector<void*>&, void *)
{
   ((Pixel_t*)o)->~Pixel_t();
   return 0;
}

// -- Stub functions for class FontStruct_t--
static void* constructor_224(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) FontStruct_t();
}
static void* constructor_225(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) FontStruct_t(*(const FontStruct_t*) arg[0]);
}

static void* method_226(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((FontStruct_t*)o)->operator=(*(const FontStruct_t*) arg[0]);
}
static void* destructor_227(void * o, const std::vector<void*>&, void *)
{
   ((FontStruct_t*)o)->~FontStruct_t();
   return 0;
}

// -- Stub functions for class Event_t--
static void* constructor_228(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) Event_t();
}
static void* constructor_229(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) Event_t(*(const Event_t*) arg[0]);
}

static void* method_230(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((Event_t*)o)->operator=(*(const Event_t*) arg[0]);
}
static void* destructor_231(void * o, const std::vector<void*>&, void *)
{
   ((Event_t*)o)->~Event_t();
   return 0;
}

// -- Stub functions for class Window_t--
static void* constructor_232(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) Window_t();
}
static void* constructor_233(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) Window_t(*(const Window_t*) arg[0]);
}

static void* method_234(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((Window_t*)o)->operator=(*(const Window_t*) arg[0]);
}
static void* destructor_235(void * o, const std::vector<void*>&, void *)
{
   ((Window_t*)o)->~Window_t();
   return 0;
}

// -- Stub functions for class GContext_t--
static void* constructor_236(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) GContext_t();
}
static void* constructor_237(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) GContext_t(*(const GContext_t*) arg[0]);
}

static void* method_238(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((GContext_t*)o)->operator=(*(const GContext_t*) arg[0]);
}
static void* destructor_239(void * o, const std::vector<void*>&, void *)
{
   ((GContext_t*)o)->~GContext_t();
   return 0;
}
} // unnamed namespace

// -------------------- Class dictionaries ---------------------------------
// --------------------- Dictionary instances ------------------------------
namespace {
struct Dictionaries {
   Dictionaries() {
      RegisterFunctionStub("operator new(unsigned int)", method_0);
      RegisterFunctionStub("operator new[](unsigned int)", method_1);
      RegisterFunctionStub("operator delete(void *)", method_2);
      RegisterFunctionStub("operator delete[](void *)", method_3);
      RegisterFunctionStub("TGSplitButton::TGSplitButton(class TGSplitButton const &)", constructor_4);
      RegisterFunctionStub("TGSplitButton::operator=(class TGSplitButton const &)", method_5);
      RegisterFunctionStub("TGSplitButton::CalcSize()", method_6);
      RegisterFunctionStub("TGSplitButton::DrawTriangle(class GContext_t const,Int_t,Int_t)", method_7);
      RegisterFunctionStub("TGSplitButton::HandleSButton(class Event_t *)", method_8);
      RegisterFunctionStub("TGSplitButton::HandleSCrossing(class Event_t *)", method_9);
      RegisterFunctionStub("TGSplitButton::HandleSKey(class Event_t *)", method_10);
      RegisterFunctionStub("TGSplitButton::SetMenuState(Bool_t)", method_11);
      RegisterFunctionStub("TGSplitButton::DoRedraw()", method_12);
      RegisterFunctionStub("TGSplitButton::Init()", method_13);
      RegisterFunctionStub("TGSplitButton::BindKeys(Bool_t)", method_14);
      RegisterFunctionStub("TGSplitButton::BindMenuKeys(Bool_t)", method_15);
      RegisterFunctionStub("TGSplitButton::TGSplitButton(class TGWindow const *,class TGHotString *,class TGPopupMenu *,Bool_t,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_16);
      RegisterFunctionStub("TGSplitButton::~TGSplitButton()", destructor_17);
      RegisterFunctionStub("TGSplitButton::GetDefaultSize()", method_18);
      RegisterFunctionStub("TGSplitButton::SetText(class TGHotString *)", method_19);
      RegisterFunctionStub("TGSplitButton::SetText(class TString const &)", method_20);
      RegisterFunctionStub("TGSplitButton::SetFont(class FontStruct_t,Bool_t)", method_21);
      RegisterFunctionStub("TGSplitButton::SetFont(char const *,Bool_t)", method_22);
      RegisterFunctionStub("TGSplitButton::SetMBState(enum EButtonState)", method_23);
      RegisterFunctionStub("TGSplitButton::SetSplit(Bool_t)", method_24);
      RegisterFunctionStub("TGSplitButton::IsSplit()", method_25);
      RegisterFunctionStub("TGSplitButton::HandleButton(class Event_t *)", method_26);
      RegisterFunctionStub("TGSplitButton::HandleCrossing(class Event_t *)", method_27);
      RegisterFunctionStub("TGSplitButton::HandleKey(class Event_t *)", method_28);
      RegisterFunctionStub("TGSplitButton::HandleMotion(class Event_t *)", method_29);
      RegisterFunctionStub("TGSplitButton::Layout()", method_30);
      RegisterFunctionStub("TGSplitButton::MBPressed()", method_31);
      RegisterFunctionStub("TGSplitButton::MBReleased()", method_32);
      RegisterFunctionStub("TGSplitButton::MBClicked()", method_33);
      RegisterFunctionStub("TGSplitButton::ItemClicked(Int_t)", method_34);
      RegisterFunctionStub("TGSplitButton::HandleMenu(Int_t)", method_35);
      RegisterFunctionStub("TGRadioButton::TGRadioButton(class TGRadioButton const &)", constructor_36);
      RegisterFunctionStub("TGRadioButton::operator=(class TGRadioButton const &)", method_37);
      RegisterFunctionStub("TGRadioButton::Init()", method_38);
      RegisterFunctionStub("TGRadioButton::PSetState(enum EButtonState,Bool_t)", method_39);
      RegisterFunctionStub("TGRadioButton::DoRedraw()", method_40);
      RegisterFunctionStub("TGRadioButton::EmitSignals(Bool_t)", method_41);
      RegisterFunctionStub("TGRadioButton::GetDefaultFontStruct()", method_42);
      RegisterFunctionStub("TGRadioButton::GetDefaultGC()", method_43);
      RegisterFunctionStub("TGRadioButton::TGRadioButton(class TGWindow const *,class TGHotString *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_44);
      RegisterFunctionStub("TGRadioButton::TGRadioButton(class TGWindow const *,char const *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_45);
      RegisterFunctionStub("TGRadioButton::TGRadioButton(class TGWindow const *,char const *,char const *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_46);
      RegisterFunctionStub("TGRadioButton::~TGRadioButton()", destructor_47);
      RegisterFunctionStub("TGRadioButton::GetDefaultSize()", method_48);
      RegisterFunctionStub("TGRadioButton::HandleButton(class Event_t *)", method_49);
      RegisterFunctionStub("TGRadioButton::HandleKey(class Event_t *)", method_50);
      RegisterFunctionStub("TGRadioButton::HandleCrossing(class Event_t *)", method_51);
      RegisterFunctionStub("TGRadioButton::SetState(enum EButtonState,Bool_t)", method_52);
      RegisterFunctionStub("TGRadioButton::SetDisabledAndSelected(Bool_t)", method_53);
      RegisterFunctionStub("TGRadioButton::IsToggleButton()", method_54);
      RegisterFunctionStub("TGRadioButton::IsExclusiveToggle()", method_55);
      RegisterFunctionStub("TGRadioButton::IsOn()", method_56);
      RegisterFunctionStub("TGRadioButton::IsDown()", method_57);
      RegisterFunctionStub("TGRadioButton::IsDisabledAndSelected()", method_58);
      RegisterFunctionStub("TGRadioButton::SavePrimitive(class ostream &,Option_t *)", method_59);
      RegisterFunctionStub("TGCheckButton::TGCheckButton(class TGCheckButton const &)", constructor_60);
      RegisterFunctionStub("TGCheckButton::operator=(class TGCheckButton const &)", method_61);
      RegisterFunctionStub("TGCheckButton::Init()", method_62);
      RegisterFunctionStub("TGCheckButton::PSetState(enum EButtonState,Bool_t)", method_63);
      RegisterFunctionStub("TGCheckButton::DoRedraw()", method_64);
      RegisterFunctionStub("TGCheckButton::EmitSignals(Bool_t)", method_65);
      RegisterFunctionStub("TGCheckButton::GetDefaultFontStruct()", method_66);
      RegisterFunctionStub("TGCheckButton::GetDefaultGC()", method_67);
      RegisterFunctionStub("TGCheckButton::TGCheckButton(class TGWindow const *,class TGHotString *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_68);
      RegisterFunctionStub("TGCheckButton::TGCheckButton(class TGWindow const *,char const *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_69);
      RegisterFunctionStub("TGCheckButton::TGCheckButton(class TGWindow const *,char const *,char const *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_70);
      RegisterFunctionStub("TGCheckButton::~TGCheckButton()", destructor_71);
      RegisterFunctionStub("TGCheckButton::GetDefaultSize()", method_72);
      RegisterFunctionStub("TGCheckButton::HandleButton(class Event_t *)", method_73);
      RegisterFunctionStub("TGCheckButton::HandleKey(class Event_t *)", method_74);
      RegisterFunctionStub("TGCheckButton::HandleCrossing(class Event_t *)", method_75);
      RegisterFunctionStub("TGCheckButton::IsToggleButton()", method_76);
      RegisterFunctionStub("TGCheckButton::IsOn()", method_77);
      RegisterFunctionStub("TGCheckButton::IsDown()", method_78);
      RegisterFunctionStub("TGCheckButton::IsDisabledAndSelected()", method_79);
      RegisterFunctionStub("TGCheckButton::SetDisabledAndSelected(Bool_t)", method_80);
      RegisterFunctionStub("TGCheckButton::SetState(enum EButtonState,Bool_t)", method_81);
      RegisterFunctionStub("TGCheckButton::SavePrimitive(class ostream &,Option_t *)", method_82);
      RegisterFunctionStub("TGPictureButton::DoRedraw()", method_83);
      RegisterFunctionStub("TGPictureButton::CreateDisabledPicture()", method_84);
      RegisterFunctionStub("TGPictureButton::TGPictureButton(class TGPictureButton const &)", constructor_85);
      RegisterFunctionStub("TGPictureButton::operator=(class TGPictureButton const &)", method_86);
      RegisterFunctionStub("TGPictureButton::TGPictureButton(class TGWindow const *,class TGPicture const *,Int_t,class GContext_t,UInt_t)", constructor_87);
      RegisterFunctionStub("TGPictureButton::TGPictureButton(class TGWindow const *,class TGPicture const *,char const *,Int_t,class GContext_t,UInt_t)", constructor_88);
      RegisterFunctionStub("TGPictureButton::TGPictureButton(class TGWindow const *,char const *,Int_t,class GContext_t,UInt_t)", constructor_89);
      RegisterFunctionStub("TGPictureButton::~TGPictureButton()", destructor_90);
      RegisterFunctionStub("TGPictureButton::SetPicture(class TGPicture const *)", method_91);
      RegisterFunctionStub("TGPictureButton::SetDisabledPicture(class TGPicture const *)", method_92);
      RegisterFunctionStub("TGPictureButton::GetPicture()", method_93);
      RegisterFunctionStub("TGPictureButton::GetDisabledPicture()", method_94);
      RegisterFunctionStub("TGPictureButton::SavePrimitive(class ostream &,Option_t *)", method_95);
      RegisterFunctionStub("TGTextButton::Init()", method_96);
      RegisterFunctionStub("TGTextButton::DoRedraw()", method_97);
      RegisterFunctionStub("TGTextButton::TGTextButton(class TGTextButton const &)", constructor_98);
      RegisterFunctionStub("TGTextButton::operator=(class TGTextButton const &)", method_99);
      RegisterFunctionStub("TGTextButton::GetDefaultFontStruct()", method_100);
      RegisterFunctionStub("TGTextButton::TGTextButton(class TGWindow const *,class TGHotString *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_101);
      RegisterFunctionStub("TGTextButton::TGTextButton(class TGWindow const *,char const *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_102);
      RegisterFunctionStub("TGTextButton::TGTextButton(class TGWindow const *,char const *,char const *,Int_t,class GContext_t,class FontStruct_t,UInt_t)", constructor_103);
      RegisterFunctionStub("TGTextButton::~TGTextButton()", destructor_104);
      RegisterFunctionStub("TGTextButton::GetDefaultSize()", method_105);
      RegisterFunctionStub("TGTextButton::HandleKey(class Event_t *)", method_106);
      RegisterFunctionStub("TGTextButton::GetText()", method_107);
      RegisterFunctionStub("TGTextButton::GetTitle()", method_108);
      RegisterFunctionStub("TGTextButton::GetString()", method_109);
      RegisterFunctionStub("TGTextButton::SetTextJustify(Int_t)", method_110);
      RegisterFunctionStub("TGTextButton::GetTextJustify()", method_111);
      RegisterFunctionStub("TGTextButton::SetText(class TGHotString *)", method_112);
      RegisterFunctionStub("TGTextButton::SetText(class TString const &)", method_113);
      RegisterFunctionStub("TGTextButton::SetTitle(char const *)", method_114);
      RegisterFunctionStub("TGTextButton::SetFont(class FontStruct_t,Bool_t)", method_115);
      RegisterFunctionStub("TGTextButton::SetFont(char const *,Bool_t)", method_116);
      RegisterFunctionStub("TGTextButton::SetTextColor(class Pixel_t,Bool_t)", method_117);
      RegisterFunctionStub("TGTextButton::SetForegroundColor(class Pixel_t)", method_118);
      RegisterFunctionStub("TGTextButton::HasOwnFont()", method_119);
      RegisterFunctionStub("TGTextButton::SetWrapLength(Int_t)", method_120);
      RegisterFunctionStub("TGTextButton::GetWrapLength()", method_121);
      RegisterFunctionStub("TGTextButton::SetMargins(Int_t,Int_t,Int_t,Int_t)", method_122);
      RegisterFunctionStub("TGTextButton::SetLeftMargin(Int_t)", method_123);
      RegisterFunctionStub("TGTextButton::SetRightMargin(Int_t)", method_124);
      RegisterFunctionStub("TGTextButton::SetTopMargin(Int_t)", method_125);
      RegisterFunctionStub("TGTextButton::SetBottomMargin(Int_t)", method_126);
      RegisterFunctionStub("TGTextButton::GetLeftMargin()", method_127);
      RegisterFunctionStub("TGTextButton::GetRightMargin()", method_128);
      RegisterFunctionStub("TGTextButton::GetTopMargin()", method_129);
      RegisterFunctionStub("TGTextButton::GetBottomMargin()", method_130);
      RegisterFunctionStub("TGTextButton::ChangeText(char const *)", method_131);
      RegisterFunctionStub("TGTextButton::GetFontStruct()", method_132);
      RegisterFunctionStub("TGTextButton::Layout()", method_133);
      RegisterFunctionStub("TGTextButton::SavePrimitive(class ostream &,Option_t *)", method_134);
      RegisterFunctionStub("TGButton::SetToggleButton(Bool_t)", method_135);
      RegisterFunctionStub("TGButton::EmitSignals(Bool_t)", method_136);
      RegisterFunctionStub("TGButton::TGButton(class TGButton const &)", constructor_137);
      RegisterFunctionStub("TGButton::operator=(class TGButton const &)", method_138);
      RegisterFunctionStub("TGButton::GetDefaultGC()", method_139);
      RegisterFunctionStub("TGButton::GetHibckgndGC()", method_140);
      RegisterFunctionStub("TGButton::TGButton(class TGWindow const *,Int_t,class GContext_t,UInt_t)", constructor_141);
      RegisterFunctionStub("TGButton::~TGButton()", destructor_142);
      RegisterFunctionStub("TGButton::HandleButton(class Event_t *)", method_143);
      RegisterFunctionStub("TGButton::HandleCrossing(class Event_t *)", method_144);
      RegisterFunctionStub("TGButton::SetUserData(void *)", method_145);
      RegisterFunctionStub("TGButton::GetUserData()", method_146);
      RegisterFunctionStub("TGButton::SetToolTipText(char const *,Long_t)", method_147);
      RegisterFunctionStub("TGButton::GetToolTip()", method_148);
      RegisterFunctionStub("TGButton::SetState(enum EButtonState,Bool_t)", method_149);
      RegisterFunctionStub("TGButton::GetState()", method_150);
      RegisterFunctionStub("TGButton::AllowStayDown(Bool_t)", method_151);
      RegisterFunctionStub("TGButton::SetGroup(class TGButtonGroup *)", method_152);
      RegisterFunctionStub("TGButton::GetGroup()", method_153);
      RegisterFunctionStub("TGButton::IsDown()", method_154);
      RegisterFunctionStub("TGButton::SetDown(Bool_t,Bool_t)", method_155);
      RegisterFunctionStub("TGButton::IsOn()", method_156);
      RegisterFunctionStub("TGButton::SetOn(Bool_t,Bool_t)", method_157);
      RegisterFunctionStub("TGButton::IsToggleButton()", method_158);
      RegisterFunctionStub("TGButton::IsExclusiveToggle()", method_159);
      RegisterFunctionStub("TGButton::Toggle(Bool_t)", method_160);
      RegisterFunctionStub("TGButton::SetEnabled(Bool_t)", method_161);
      RegisterFunctionStub("TGButton::SavePrimitive(class ostream &,Option_t *)", method_162);
      RegisterFunctionStub("TGButton::GetNormGC()", method_163);
      RegisterFunctionStub("TGButton::Pressed()", method_164);
      RegisterFunctionStub("TGButton::Released()", method_165);
      RegisterFunctionStub("TGButton::Clicked()", method_166);
      RegisterFunctionStub("TGButton::Toggled(Bool_t)", method_167);
      RegisterFunctionStub("ostream::ostream()", constructor_168);
      RegisterFunctionStub("ostream::ostream(class ostream const &)", constructor_169);
      RegisterFunctionStub("ostream::operator=(class ostream const &)", method_170);
      RegisterFunctionStub("ostream::~ostream()", destructor_171);
      RegisterFunctionStub("TGButtonGroup::TGButtonGroup()", constructor_172);
      RegisterFunctionStub("TGButtonGroup::TGButtonGroup(class TGButtonGroup const &)", constructor_173);
      RegisterFunctionStub("TGButtonGroup::operator=(class TGButtonGroup const &)", method_174);
      RegisterFunctionStub("TGButtonGroup::~TGButtonGroup()", destructor_175);
      RegisterFunctionStub("TGFrame::TGFrame()", constructor_176);
      RegisterFunctionStub("TGFrame::TGFrame(class TGFrame const &)", constructor_177);
      RegisterFunctionStub("TGFrame::operator=(class TGFrame const &)", method_178);
      RegisterFunctionStub("TGFrame::~TGFrame()", destructor_179);
      RegisterFunctionStub("TGWindow::TGWindow()", constructor_180);
      RegisterFunctionStub("TGWindow::TGWindow(class TGWindow const &)", constructor_181);
      RegisterFunctionStub("TGWindow::operator=(class TGWindow const &)", method_182);
      RegisterFunctionStub("TGWindow::~TGWindow()", destructor_183);
      RegisterFunctionStub("TGWidget::Emit(char const *,_Bool)", method_184);
      RegisterFunctionStub("TGWidget::TGWidget()", constructor_185);
      RegisterFunctionStub("TGWidget::TGWidget(class TGWidget const &)", constructor_186);
      RegisterFunctionStub("TGWidget::operator=(class TGWidget const &)", method_187);
      RegisterFunctionStub("TGWidget::~TGWidget()", destructor_188);
      RegisterFunctionStub("TGGC::operator()()", method_189);
      RegisterFunctionStub("TGGC::TGGC()", constructor_190);
      RegisterFunctionStub("TGGC::TGGC(class TGGC const &)", constructor_191);
      RegisterFunctionStub("TGGC::operator=(class TGGC const &)", method_192);
      RegisterFunctionStub("TGGC::~TGGC()", destructor_193);
      RegisterFunctionStub("TGHotString::TGHotString(char const *)", constructor_194);
      RegisterFunctionStub("TGHotString::TGHotString(class TGHotString const &)", constructor_195);
      RegisterFunctionStub("TGHotString::operator=(class TGHotString const &)", method_196);
      RegisterFunctionStub("TGHotString::~TGHotString()", destructor_197);
      RegisterFunctionStub("TGString::TGString(char const *)", constructor_198);
      RegisterFunctionStub("TGString::Data()", method_199);
      RegisterFunctionStub("TGString::GetString()", method_200);
      RegisterFunctionStub("TGString::TGString(class TGString const &)", constructor_201);
      RegisterFunctionStub("TGString::operator=(class TGString const &)", method_202);
      RegisterFunctionStub("TGString::~TGString()", destructor_203);
      RegisterFunctionStub("TString::TString(char const *)", constructor_204);
      RegisterFunctionStub("TString::TString(class TString const &)", constructor_205);
      RegisterFunctionStub("TString::operator=(class TString const &)", method_206);
      RegisterFunctionStub("TString::~TString()", destructor_207);
      RegisterFunctionStub("TGDimension::TGDimension()", constructor_208);
      RegisterFunctionStub("TGDimension::TGDimension(class TGDimension const &)", constructor_209);
      RegisterFunctionStub("TGDimension::operator=(class TGDimension const &)", method_210);
      RegisterFunctionStub("TGDimension::~TGDimension()", destructor_211);
      RegisterFunctionStub("TGFont::TGFont()", constructor_212);
      RegisterFunctionStub("TGFont::TGFont(class TGFont const &)", constructor_213);
      RegisterFunctionStub("TGFont::operator=(class TGFont const &)", method_214);
      RegisterFunctionStub("TGFont::~TGFont()", destructor_215);
      RegisterFunctionStub("Cursor_t::Cursor_t()", constructor_216);
      RegisterFunctionStub("Cursor_t::Cursor_t(class Cursor_t const &)", constructor_217);
      RegisterFunctionStub("Cursor_t::operator=(class Cursor_t const &)", method_218);
      RegisterFunctionStub("Cursor_t::~Cursor_t()", destructor_219);
      RegisterFunctionStub("Pixel_t::Pixel_t()", constructor_220);
      RegisterFunctionStub("Pixel_t::Pixel_t(class Pixel_t const &)", constructor_221);
      RegisterFunctionStub("Pixel_t::operator=(class Pixel_t const &)", method_222);
      RegisterFunctionStub("Pixel_t::~Pixel_t()", destructor_223);
      RegisterFunctionStub("FontStruct_t::FontStruct_t()", constructor_224);
      RegisterFunctionStub("FontStruct_t::FontStruct_t(class FontStruct_t const &)", constructor_225);
      RegisterFunctionStub("FontStruct_t::operator=(class FontStruct_t const &)", method_226);
      RegisterFunctionStub("FontStruct_t::~FontStruct_t()", destructor_227);
      RegisterFunctionStub("Event_t::Event_t()", constructor_228);
      RegisterFunctionStub("Event_t::Event_t(class Event_t const &)", constructor_229);
      RegisterFunctionStub("Event_t::operator=(class Event_t const &)", method_230);
      RegisterFunctionStub("Event_t::~Event_t()", destructor_231);
      RegisterFunctionStub("Window_t::Window_t()", constructor_232);
      RegisterFunctionStub("Window_t::Window_t(class Window_t const &)", constructor_233);
      RegisterFunctionStub("Window_t::operator=(class Window_t const &)", method_234);
      RegisterFunctionStub("Window_t::~Window_t()", destructor_235);
      RegisterFunctionStub("GContext_t::GContext_t()", constructor_236);
      RegisterFunctionStub("GContext_t::GContext_t(class GContext_t const &)", constructor_237);
      RegisterFunctionStub("GContext_t::operator=(class GContext_t const &)", method_238);
      RegisterFunctionStub("GContext_t::~GContext_t()", destructor_239);
   }
   ~Dictionaries() {
   }
};
static Dictionaries instance;
}

// End of Dictionary
