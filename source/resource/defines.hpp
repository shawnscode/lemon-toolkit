// @date 2016/07/26
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

#define NS_FLOW2D_RES_BEGIN namespace flow2d { namespace res {
#define NS_FLOW2D_RES_END } }

#define NS_FLOW2D_FS_BEGIN namespace flow2d { namespace fs {
#define NS_FLOW2D_FS_END }}

NS_FLOW2D_RES_BEGIN

NS_FLOW2D_RES_END

NS_FLOW2D_FS_BEGIN

const static unsigned kMaxPath = 256;
struct File;
struct Filesystem;

NS_FLOW2D_FS_END