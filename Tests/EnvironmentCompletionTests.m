#import "../Sources/TSAutomaticEnvironment.h"
int main(void) { @autoreleasepool {
    NSArray *cases = @[
        @[@"\\begin{document}", @YES], @[@"\t  \\begin{itemize}", @YES],
        @[@"\\begin{align*}", @YES], @[@"\\begin{figure}[htbp]", @YES],
        @[@"\\begin{tabular}{ll}", @YES], @[@"% \\begin{document}", @NO],
        @[@"\\\\begin{document}", @NO], @[@"\\begin{}", @NO],
        @[@"text \\begin{document}", @NO], @[@"\\begin{document", @NO]];
    for (NSArray *c in cases) {
        NSString *text = c[0];
        NSDictionary *result = TSAutomaticEnvironmentInsertion(text, NSMakeRange(text.length, 0), '\\');
        NSCAssert((result != nil) == [c[1] boolValue], @"opening case: %@", text);
    }
    NSString *opening = @"\\begin{document}";
    NSArray *tails = @[@"\n\\end{document}", @"\ntext\n\\end{document}", @"\n\\begin{document}\n\\end{document}\n\\end{document}"];
    for (NSString *tail in tails) NSCAssert(!TSAutomaticEnvironmentInsertion([opening stringByAppendingString:tail], NSMakeRange(opening.length, 0), '\\'), @"duplicate end");
    NSArray *ignored = @[@"\n% \\end{document}", @"\n\\\\end{document}", @"\n\\begin{document}\n\\end{document}"];
    for (NSString *tail in ignored) NSCAssert(TSAutomaticEnvironmentInsertion([opening stringByAppendingString:tail], NSMakeRange(opening.length, 0), '\\'), @"nested/ignored end");
    NSDictionary *result = TSAutomaticEnvironmentInsertion(@"  \\begin{document}", NSMakeRange(18, 0), '\\');
    NSCAssert([result[@"text"] isEqualToString:@"\n  \n  \\end{document}"] && [result[@"caret"] integerValue] == 3, @"indent/caret");
    NSCAssert(!TSAutomaticEnvironmentInsertion(opening, NSMakeRange(0, opening.length), '\\'), @"selection");
    NSCAssert(!TSAutomaticEnvironmentInsertion([opening stringByAppendingString:@" trailing"], NSMakeRange(opening.length, 0), '\\'), @"trailing text");
    printf("19 environment checks passed\n");
} return 0; }
