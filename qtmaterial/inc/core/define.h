#pragma once

#define SAFE_DELETE(p) do {if (p) {delete p; p = Q_NULLPTR;}} while (false)

#define SAFE_RELEASE(p) do {if (p) {(p)->Release(); p = Q_NULLPTR;}} while (false)

#define IF_TRUE_RETURN_VALUE(expr, value) do {if ((expr)) return value;} while (false)
#define IF_FALSE_RETURN_VALUE(expr, value) FF_IF_TRUE_RETURN_VALUE(!(expr), value)
#define IF_NULL_RETURN_VALUE(expr, value) FF_IF_FALSE_RETURN_VALUE(expr, value)

#define IF_FALSE_RETURN_VALUE_ASSERT(expr, value) do {if (!(expr)) { Q_ASSERT(false); return value; }} while (false)
#define IF_NULL_RETURN_VALUE_ASSERT(expr, value) FF_IF_FALSE_RETURN_VALUE_ASSERT(expr, value)

#define IF_TRUE_RETURN_VOID(expr) do {if ((expr)) return;} while (false)
#define IF_FALSE_RETURN_VOID(expr) do {if (!(expr)) return;} while (false)
#define IF_NULL_RETURN_VOID(expr) FF_IF_FALSE_RETURN_VOID(expr) 

#define IF_FALSE_RETURN_VOID_ASSERT(expr) do {if (!(expr)) { Q_ASSERT(false); return; }} while (false)
#define IF_NULL_RETURN_VOID_ASSERT(expr) FF_IF_FALSE_RETURN_VOID_ASSERT(expr)