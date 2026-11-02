#import <Foundation/Foundation.h>

static NSString * const TSBuildOnSavePreference = @"TSBuildOnSaveEnabled";

// A compiler directive selects the compiler, not a single-pass legacy recipe,
// when the user is in AutoLaTeX mode. Custom engines retain their normal meaning.
static inline NSString *TSWorkflowProgram(NSString *program, BOOL automatic)
{
    NSString *name = program.lowercaseString;
    if (automatic && [@[@"pdflatex", @"xelatex", @"lualatex", @"pdflatexmk", @"xelatexmk", @"lualatexmk"] containsObject:name]) return @"autolatex";
    return name;
}
