// Small, dependency-free source transformation shared by the editor and native tests.
#import <Foundation/Foundation.h>

static inline BOOL TSPositionIsCommented(NSString *text, NSUInteger position)
{
    NSUInteger start = [text lineRangeForRange:NSMakeRange(position, 0)].location;
    NSUInteger escapes = 0;
    for (NSUInteger i = start; i < position; i++) {
        unichar c = [text characterAtIndex:i];
        if (c == '%' && !(escapes % 2)) return YES;
        escapes = c == '\\' ? escapes + 1 : 0;
    }
    return NO;
}

// Returns the text to insert and the caret offset, or nil for an ordinary Return.
static inline NSDictionary *TSAutomaticEnvironmentInsertion(NSString *source, NSRange selection, unichar texCharacter)
{
    if (selection.length || selection.location > source.length) return nil;
    NSString *text = source;
    if (texCharacter != '\\') text = [text stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", texCharacter] withString:@"\\"];
    NSRange line = [text lineRangeForRange:selection];
    NSString *prefix = [text substringWithRange:NSMakeRange(line.location, selection.location - line.location)];
    NSRegularExpression *opening = [NSRegularExpression regularExpressionWithPattern:@"^([ \\t]*)\\\\begin\\{([A-Za-z][A-Za-z0-9*@._:-]*)\\}([ \\t]*(?:\\[[^\\]\\r\\n]*\\]|\\{[^{}\\r\\n]*\\})[ \\t]*)*$" options:0 error:NULL];
    NSTextCheckingResult *match = [opening firstMatchInString:prefix options:0 range:NSMakeRange(0, prefix.length)];
    if (!match) return nil;
    // Do not split trailing source text or comments into the generated environment.
    NSString *tail = [text substringWithRange:NSMakeRange(selection.location, NSMaxRange(line) - selection.location)];
    if ([[tail stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] length]) return nil;
    NSString *indent = [prefix substringWithRange:[match rangeAtIndex:1]];
    NSString *name = [prefix substringWithRange:[match rangeAtIndex:2]];
    NSString *pattern = [NSString stringWithFormat:@"\\\\(begin|end)\\{%@\\}", [NSRegularExpression escapedPatternForString:name]];
    NSRegularExpression *tokens = [NSRegularExpression regularExpressionWithPattern:pattern options:0 error:NULL];
    NSInteger depth = 1;
    for (NSTextCheckingResult *token in [tokens matchesInString:text options:0 range:NSMakeRange(selection.location, text.length - selection.location)]) {
        NSUInteger pos = token.range.location, escapes = 0;
        while (pos > 0 && [text characterAtIndex:pos - 1] == '\\') { pos--; escapes++; }
        if (escapes % 2 || TSPositionIsCommented(text, token.range.location)) continue;
        if ([[text substringWithRange:[token rangeAtIndex:1]] isEqualToString:@"begin"]) depth++;
        else if (--depth == 0) return nil;
    }
    NSString *insertion = [NSString stringWithFormat:@"\n%@\n%@%Cend{%@}", indent, indent, texCharacter, name];
    return @{@"text": insertion, @"caret": @(1 + indent.length)};
}
