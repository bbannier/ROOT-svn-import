@class RangeSlider;

@interface H1BinsInspector : UIViewController {
   RangeSlider *axisRangeSlider;
   
   IBOutlet UILabel *minLabel;
   IBOutlet UILabel *maxLabel;
}

@property (nonatomic, retain) UILabel *minLabel;
@property (nonatomic, retain) UILabel *maxLabel;

@end
