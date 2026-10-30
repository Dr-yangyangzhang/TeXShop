#import <Foundation/Foundation.h>
#import "TSAutomaticEnvironment.h"

// Bounded, current-line context detection. No disk access in the typing path.
static inline NSDictionary *TSArgumentCompletion(NSString *source, NSRange selection, unichar texCharacter)
{
    if (selection.length || selection.location > source.length) return nil;
    NSString *text = texCharacter == '\\' ? source : [source stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%C", texCharacter] withString:@"\\"];
    if (TSPositionIsCommented(text, selection.location)) return nil;
    NSUInteger start = [text lineRangeForRange:selection].location;
    start = MAX(start, selection.location > 512 ? selection.location - 512 : 0);
    NSString *before = [text substringWithRange:NSMakeRange(start, selection.location - start)];
    NSRegularExpression *pattern = [NSRegularExpression regularExpressionWithPattern:@"\\\\(documentclass|usepackage|RequirePackage|begin|end|bibliographystyle|ref|eqref|pageref|autoref)\\*?(?:\\[[^\\]\\r\\n]*\\])?\\{([^{}\\\\\\r\\n]*)$" options:0 error:NULL];
    NSTextCheckingResult *match = [pattern firstMatchInString:before options:0 range:NSMakeRange(0, before.length)];
    if (!match) return nil;
    NSUInteger slash = start + match.range.location, escapes = 0;
    while (slash > 0 && [text characterAtIndex:slash - 1] == '\\') { slash--; escapes++; }
    if (escapes % 2) return nil;
    NSString *kind = [before substringWithRange:[match rangeAtIndex:1]];
    NSRange argument = [match rangeAtIndex:2];
    NSString *partial = [before substringWithRange:argument];
    NSRange comma = [partial rangeOfString:@"," options:NSBackwardsSearch];
    if (comma.location != NSNotFound) {
        if (![kind isEqualToString:@"usepackage"] && ![kind isEqualToString:@"RequirePackage"]) return nil;
        argument.location += comma.location + 1;
        argument.length -= comma.location + 1;
    }
    while (argument.length && [[NSCharacterSet whitespaceCharacterSet] characterIsMember:[before characterAtIndex:argument.location]]) { argument.location++; argument.length--; }
    NSArray *candidates;
    if ([kind isEqualToString:@"documentclass"]) candidates = @[@"article", @"report", @"book", @"letter", @"slides", @"beamer", @"memoir", @"scrartcl", @"scrreprt", @"scrbook", @"standalone", @"amsart", @"amsbook", @"IEEEtran"];
    else if ([kind isEqualToString:@"begin"] || [kind isEqualToString:@"end"]) candidates = @[@"document", @"abstract", @"itemize", @"enumerate", @"description", @"equation", @"equation*", @"align", @"align*", @"aligned", @"gather", @"gather*", @"figure", @"figure*", @"table", @"table*", @"tabular", @"tabular*", @"tabularx", @"center", @"flushleft", @"flushright", @"minipage", @"verbatim", @"quote", @"quotation", @"theorem", @"proof", @"frame", @"tikzpicture", @"cases", @"matrix", @"pmatrix", @"bmatrix"];
    else if ([kind isEqualToString:@"bibliographystyle"]) candidates = @[@"plain", @"unsrt", @"alpha", @"abbrv", @"plainnat", @"unsrtnat", @"abbrvnat", @"IEEEtran"];
    else if ([kind isEqualToString:@"usepackage"] || [kind isEqualToString:@"RequirePackage"]) candidates = @[@"amsmath", @"amssymb", @"amsthm", @"graphicx", @"geometry", @"hyperref", @"cleveref", @"xcolor", @"booktabs", @"array", @"tabularx", @"longtable", @"enumitem", @"mathtools", @"fontspec", @"unicode-math", @"biblatex", @"natbib", @"tikz", @"pgfplots", @"microtype", @"siunitx", @"subcaption", @"caption", @"babel", @"polyglossia", @"ctex", @"xeCJK", @"inputenc", @"fontenc"];
    else {
        NSMutableSet *labels = [NSMutableSet set];
        NSRegularExpression *labelPattern = [NSRegularExpression regularExpressionWithPattern:@"\\\\label\\{([^{}]+)\\}" options:0 error:NULL];
        for (NSTextCheckingResult *label in [labelPattern matchesInString:text options:0 range:NSMakeRange(0, text.length)]) {
            NSUInteger pos = label.range.location, count = 0;
            while (pos > 0 && [text characterAtIndex:pos - 1] == '\\') { pos--; count++; }
            if (!(count % 2) && !TSPositionIsCommented(text, label.range.location)) [labels addObject:[text substringWithRange:[label rangeAtIndex:1]]];
        }
        candidates = [labels allObjects];
    }
    NSString *prefix = [before substringWithRange:argument];
    NSMutableArray *matches = [NSMutableArray array];
    for (NSString *value in candidates) if (!prefix.length || [value hasPrefix:prefix]) [matches addObject:value];
    // Replace the rest of the token too when accepting in the middle of a value.
    NSUInteger end = selection.location;
    NSCharacterSet *boundaries = [NSCharacterSet characterSetWithCharactersInString:@"{}[],\\ \t\r\n"];
    while (end < text.length && ![boundaries characterIsMember:[text characterAtIndex:end]]) end++;
    return @{@"range": [NSValue valueWithRange:NSMakeRange(start + argument.location, end - start - argument.location)], @"values": [matches sortedArrayUsingSelector:@selector(compare:) ]};
}
