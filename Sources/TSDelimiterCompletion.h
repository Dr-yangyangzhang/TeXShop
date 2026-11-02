#import <Foundation/Foundation.h>
#import "TSAutomaticEnvironment.h"

// Minimal dollar-math state for delimiter insertion; does not evaluate TeX macros.
static inline NSUInteger TSDollarMathState(NSString *source, NSUInteger end, unichar texCharacter)
{
    NSUInteger state = 0, escapes = 0;
    BOOL comment = NO;
    for (NSUInteger i = 0; i < end; i++) {
        unichar c = [source characterAtIndex:i];
        if (c == '\n' || c == '\r') { comment = NO; escapes = 0; continue; }
        if (comment) continue;
        if (c == '%' && !(escapes % 2)) { comment = YES; escapes = 0; continue; }
        if (c == '$' && !(escapes % 2)) {
            BOOL display = i + 1 < end && [source characterAtIndex:i + 1] == '$';
            if (!state) state = display ? 2 : 1;
            else if (state == 1 && !display) state = 0;
            else if (state == 2 && display) state = 0;
            if (display) i++;
        }
        escapes = c == texCharacter ? escapes + 1 : 0;
    }
    return state;
}

// Returns one replacement and its resulting selection, or nil for ordinary input.
static inline NSDictionary *TSDelimiterCompletion(NSString *source, NSRange range, NSString *typed, unichar texCharacter)
{
    if (typed.length != 1 || range.location > source.length || range.length > source.length - range.location) return nil;
    NSDictionary *pairs = @{@"{": @"}", @"(": @")", @"[": @"]", @"$": @"$"};
    NSString *closing = pairs[typed];
    if (!closing && ![@[@"}", @")", @"]"] containsObject:typed]) return nil;
    if (range.length) {
        if (!closing) return nil;
        NSString *selected = [source substringWithRange:range];
        return @{@"text": [NSString stringWithFormat:@"%@%@%@", typed, selected, closing],
                 @"selection": [NSValue valueWithRange:NSMakeRange(range.location + 1, range.length)]};
    }
    NSUInteger escapes = 0, pos = range.location;
    while (pos && [source characterAtIndex:pos - 1] == texCharacter) { escapes++; pos--; }
    if (escapes % 2) return nil; // Preserve literal \$, \{, \(, etc.
    NSString *normalized = texCharacter == '\\' ? source : [source stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", texCharacter] withString:@"\\"];
    if (TSPositionIsCommented(normalized, range.location)) return nil;
    BOOL nextMatches = range.location < source.length && [source characterAtIndex:range.location] == [typed characterAtIndex:0];
    if ([typed isEqualToString:@"$"]) {
        NSUInteger mathState = TSDollarMathState(source, range.location, texCharacter);
        // A second opener promotes the freshly empty inline pair to display math.
        if (mathState == 1 && nextMatches && range.location && [source characterAtIndex:range.location - 1] == '$') {
            return @{@"text": @"$$", @"selection": [NSValue valueWithRange:NSMakeRange(range.location + 1, 0)]};
        }
        if (mathState && nextMatches) return @{@"skip": @YES, @"selection": [NSValue valueWithRange:NSMakeRange(range.location + 1, 0)]};
        if (mathState) return nil; // Close existing math rather than opening another pair.
    } else if (!closing) {
        if (nextMatches) return @{@"skip": @YES, @"selection": [NSValue valueWithRange:NSMakeRange(range.location + 1, 0)]};
        return nil;
    }
    return @{@"text": [typed stringByAppendingString:closing],
             @"selection": [NSValue valueWithRange:NSMakeRange(range.location + 1, 0)]};
}
