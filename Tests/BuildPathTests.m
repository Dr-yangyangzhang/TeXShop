#import "../Sources/TSBuildPaths.h"
int main(void) { @autoreleasepool {
    NSFileManager *files = NSFileManager.defaultManager;
    NSString *folder = [NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString];
    NSString *source = [folder stringByAppendingPathComponent:@"sample.tex"];
    NSString *normal = [source stringByDeletingPathExtension];
    NSString *cached = [folder stringByAppendingPathComponent:@".texshop-build/sample/sample"];
    [files createDirectoryAtPath:cached.stringByDeletingLastPathComponent withIntermediateDirectories:YES attributes:nil error:NULL];
    NSCAssert([TSSyncTeXOutputBase(source) isEqualToString:normal], @"normal default");
    NSString *cacheSync = [cached stringByAppendingPathExtension:@"synctex.gz"];
    [@"cache" writeToFile:cacheSync atomically:YES encoding:NSUTF8StringEncoding error:NULL];
    [files setAttributes:@{NSFileModificationDate:[NSDate dateWithTimeIntervalSince1970:100]} ofItemAtPath:cacheSync error:NULL];
    NSCAssert([TSSyncTeXOutputBase(source) isEqualToString:cached], @"discover hidden sync");
    NSString *normalSync = [normal stringByAppendingPathExtension:@"synctex.gz"];
    [@"normal" writeToFile:normalSync atomically:YES encoding:NSUTF8StringEncoding error:NULL];
    [files setAttributes:@{NSFileModificationDate:[NSDate dateWithTimeIntervalSince1970:200]} ofItemAtPath:normalSync error:NULL];
    NSCAssert([TSSyncTeXOutputBase(source) isEqualToString:normal], @"newer conventional sync");
    [files setAttributes:@{NSFileModificationDate:[NSDate dateWithTimeIntervalSince1970:300]} ofItemAtPath:cacheSync error:NULL];
    NSCAssert([TSSyncTeXOutputBase(source) isEqualToString:cached], @"switch back to auto");
    NSString *log = [cached stringByAppendingPathExtension:@"log"];
    [@"log" writeToFile:log atomically:YES encoding:NSUTF8StringEncoding error:NULL];
    NSCAssert([TSBuildArtifactPath(source, @"log") isEqualToString:log], @"cached log viewer");
    printf("5 build-path checks passed\n");
} return 0; }
