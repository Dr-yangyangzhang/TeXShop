#import "../Sources/TSArgumentCompletion.h"
int main(void) { @autoreleasepool {
    NSArray *cases = @[
        @[@"\\documentclass{ar", @"article"], @[@"\\documentclass[12pt]{bo", @"book"],
        @[@"\\documentclass{", @"report"], @[@"\\usepackage{amsmath, hyp", @"hyperref"],
        @[@"\\begin{doc", @"document"], @[@"\\end{ali", @"align*"],
        @[@"\\bibliographystyle{pl", @"plain"], @[@"\\label{sec:intro}\n\\ref{sec:", @"sec:intro"]];
    for (NSArray *c in cases) {
        NSString *text = c[0];
        NSDictionary *result = TSArgumentCompletion(text, NSMakeRange(text.length, 0), '\\');
        NSCAssert([result[@"values"] containsObject:c[1]], @"missing context candidate: %@", text);
    }
    for (NSString *text in @[@"% \\documentclass{ar", @"\\\\documentclass{ar", @"ordinary text", @"\\documentclass{article,bo"]) NSCAssert(!TSArgumentCompletion(text, NSMakeRange(text.length, 0), '\\'), @"invalid context: %@", text);
    NSString *text = @"\\documentclass{ar}";
    NSDictionary *result = TSArgumentCompletion(text, NSMakeRange(text.length - 1, 0), '\\');
    NSCAssert(NSEqualRanges([result[@"range"] rangeValue], NSMakeRange(15, 2)), @"replace only argument prefix");
    NSMutableString *edited = [text mutableCopy];
    [edited replaceCharactersInRange:[result[@"range"] rangeValue] withString:@"article"];
    NSCAssert([edited isEqualToString:@"\\documentclass{article}"], @"preserve surrounding command/braces");
    NSCAssert(!TSArgumentCompletion(text, NSMakeRange(0, 3), '\\'), @"selection");
    text = @"\\documentclass{article}";
    result = TSArgumentCompletion(text, NSMakeRange(17, 0), '\\');
    NSCAssert(NSEqualRanges([result[@"range"] rangeValue], NSMakeRange(15, 7)), @"replace suffix when editing mid-token");
    printf("16 argument completion checks passed\n");
} return 0; }
