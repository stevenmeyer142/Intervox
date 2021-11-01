//
//  renderheadless_objc.m
//  examples-ios
//
//  Created by Steven Meyer on 10/30/21.
//

#import <Foundation/Foundation.h>
#include "VulkanTools.h"

const std::string getAssetPath()
{
    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
    std::string result([resourcePath UTF8String]);
    return result + "/data/";
}
