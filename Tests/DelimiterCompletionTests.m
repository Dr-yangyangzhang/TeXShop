#import "../Sources/TSDelimiterCompletion.h"
static NSString *Apply(NSString *source, NSRange range, NSString *typed, NSRange *selection)
{
    NSDictionary *edit = TSDelimiterCompletion(source, range, typed, '\\');
    NSCAssert(edit, @"Expected delimiter action");
    NSMutableString *result = source.mutableCopy;
    if (![edit[@"skip"] boolValue]) [result replaceCharactersInRange:range withString:edit[@"text"]];
    *selection = [edit[@"selection"] rangeValue];
    return result;
}
int main(void) { @autoreleasepool {
    NSRange selection;
    NSArray *open = @[@"{", @"(", @"[", @"$"];
    NSArray *close = @[@"}", @")", @"]", @"$"];
    for (NSUInteger i = 0; i < open.count; i++) {
        NSString *result = Apply(@"", NSMakeRange(0, 0), open[i], &selection);
        NSCAssert([result isEqualToString:[open[i] stringByAppendingString:close[i]]] && NSEqualRanges(selection, NSMakeRange(1, 0)), @"pair and caret");
        result = Apply(@"before α😀\nafter", NSMakeRange(7, 4), open[i], &selection);
        NSString *expected = [NSString stringWithFormat:@"before %@α😀\n%@after", open[i], close[i]];
        NSCAssert([result isEqualToString:expected] && NSEqualRanges(selection, NSMakeRange(8, 4)), @"wrap unicode and multiline selection");
    }
    NSString *result = Apply(@"{}", NSMakeRange(1, 0), @"{", &selection);
    NSCAssert([result isEqualToString:@"{{}}"] && selection.location == 2, @"nested braces");
    for (NSString *symbol in @[@"}", @")", @"]"]) {
        result = Apply(symbol, NSMakeRange(0, 0), symbol, &selection);
        NSCAssert([result isEqualToString:symbol] && selection.location == 1, @"skip closing delimiter");
    }
    result = Apply(@"$$", NSMakeRange(1, 0), @"$", &selection);
    NSCAssert([result isEqualToString:@"$$$$"] && selection.location == 2, @"display promotion");
    result = Apply(@"$x$", NSMakeRange(2, 0), @"$", &selection);
    NSCAssert([result isEqualToString:@"$x$"] && selection.location == 3, @"exit inline math");
    result = Apply(@"$$x$$", NSMakeRange(3, 0), @"$", &selection);
    result = Apply(result, selection, @"$", &selection);
    NSCAssert([result isEqualToString:@"$$x$$"] && selection.location == 5, @"exit display math");
    NSCAssert(!TSDelimiterCompletion(@"$x", NSMakeRange(2, 0), @"$", '\\'), @"close unfinished inline math normally");
    NSCAssert(!TSDelimiterCompletion(@"$$\nx", NSMakeRange(4, 0), @"$", '\\'), @"multiline display math");
    for (NSString *symbol in open) NSCAssert(!TSDelimiterCompletion(@"\\", NSMakeRange(1, 0), symbol, '\\'), @"escaped opening symbol");
    NSCAssert(TSDelimiterCompletion(@"\\\\", NSMakeRange(2, 0), @"{", '\\'), @"even backslashes");
    NSCAssert(!TSDelimiterCompletion(@"% comment", NSMakeRange(9, 0), @"{", '\\'), @"comment");
    NSCAssert(TSDelimiterCompletion(@"\\% ", NSMakeRange(3, 0), @"{", '\\'), @"escaped percent");
    NSCAssert(!TSDelimiterCompletion(@"text", NSMakeRange(0, 4), @")", '\\'), @"closing character does not wrap");
    NSCAssert(!TSDelimiterCompletion(@"", NSMakeRange(0, 0), @"paste", '\\'), @"multicharacter insertion");
    NSCAssert(!TSDelimiterCompletion(@"", NSMakeRange(2, 0), @"{", '\\'), @"invalid range");
    NSCAssert(!TSDelimiterCompletion(@"", NSMakeRange(0, 0), @"<", '\\'), @"comparison is not paired");
    NSCAssert(TSDollarMathState(@"% $\n\\$", 6, '\\') == 0, @"ignore comments and literal dollars");
    NSCAssert(TSDollarMathState(@"$a$$b$", 6, '\\') == 0, @"adjacent inline math");
    result = Apply(@"% selected", NSMakeRange(2, 8), @"{", &selection);
    NSCAssert([result isEqualToString:@"% {selected}"] && selection.length == 8, @"explicit selection wrapping also works in comments");
    result = Apply(@"\\selected", NSMakeRange(1, 8), @"[", &selection);
    NSCAssert([result isEqualToString:@"\\[selected]"] && selection.length == 8, @"explicit wrapping is not suppressed by preceding backslash");
    printf("32 delimiter checks passed\n");
} return 0; }
