#import <Foundation/Foundation.h>

// The normal SyncTeX parser can read the retained AutoLaTeX build directly.
// Prefer a newer conventional sync file after switching back to a legacy engine.
static inline NSString *TSSyncTeXOutputBase(NSString *source)
{
    NSString *normal = [source stringByDeletingPathExtension];
    NSString *stem = normal.lastPathComponent;
    NSString *cached = [[[[source stringByDeletingLastPathComponent] stringByAppendingPathComponent:@".texshop-build"] stringByAppendingPathComponent:stem] stringByAppendingPathComponent:stem];
    NSFileManager *files = [NSFileManager defaultManager];
    NSDate *cachedDate = nil, *normalDate = nil;
    for (NSString *extension in @[@"synctex", @"synctex.gz"]) {
        NSDate *date = [files attributesOfItemAtPath:[cached stringByAppendingPathExtension:extension] error:NULL][NSFileModificationDate];
        if (date && (!cachedDate || [date compare:cachedDate] == NSOrderedDescending)) cachedDate = date;
        date = [files attributesOfItemAtPath:[normal stringByAppendingPathExtension:extension] error:NULL][NSFileModificationDate];
        if (date && (!normalDate || [date compare:normalDate] == NSOrderedDescending)) normalDate = date;
    }
    return cachedDate && (!normalDate || [cachedDate compare:normalDate] != NSOrderedAscending) ? cached : normal;
}

static inline NSString *TSBuildArtifactPath(NSString *source, NSString *extension)
{
    NSString *normal = [[source stringByDeletingPathExtension] stringByAppendingPathExtension:extension];
    NSString *stem = [source.lastPathComponent stringByDeletingPathExtension];
    NSString *cached = [[[[source stringByDeletingLastPathComponent] stringByAppendingPathComponent:@".texshop-build"] stringByAppendingPathComponent:stem] stringByAppendingPathComponent:[stem stringByAppendingPathExtension:extension]];
    NSFileManager *files = NSFileManager.defaultManager;
    NSDate *cachedDate = [files attributesOfItemAtPath:cached error:NULL][NSFileModificationDate];
    NSDate *normalDate = [files attributesOfItemAtPath:normal error:NULL][NSFileModificationDate];
    return cachedDate && (!normalDate || [cachedDate compare:normalDate] != NSOrderedAscending) ? cached : normal;
}
