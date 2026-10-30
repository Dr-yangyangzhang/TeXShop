#import "../Sources/TSAutomaticBuildPolicy.h"
int main(void) { @autoreleasepool {
    for (NSString *engine in @[@"pdflatex", @"XeLaTeX", @"lualatex", @"pdflatexmk", @"xelatexmk", @"lualatexmk"]) {
        NSCAssert([TSWorkflowProgram(engine, YES) isEqualToString:@"autolatex"], @"automatic directive routing");
        NSCAssert([TSWorkflowProgram(engine, NO) isEqualToString:engine.lowercaseString], @"legacy selection");
    }
    NSCAssert([TSWorkflowProgram(@"custom-engine", YES) isEqualToString:@"custom-engine"], @"custom engine preserved");
    NSString *suite = [@"TeXShop.AutomaticBuildTests." stringByAppendingString:NSUUID.UUID.UUIDString];
    NSUserDefaults *first = [[NSUserDefaults alloc] initWithSuiteName:suite];
    NSCAssert(![first boolForKey:TSBuildOnSavePreference], @"off until enabled");
    [first setBool:YES forKey:TSBuildOnSavePreference];
    NSUserDefaults *reopened = [[NSUserDefaults alloc] initWithSuiteName:suite];
    NSCAssert([reopened boolForKey:TSBuildOnSavePreference], @"opt-in survives reopening defaults");
    [reopened setBool:NO forKey:TSBuildOnSavePreference];
    NSCAssert(![first boolForKey:TSBuildOnSavePreference], @"disable shared across windows");
    [first removePersistentDomainForName:suite];
    printf("16 automatic build policy checks passed\n");
} return 0; }
