/* SPDX-FileCopyrightText: 2012 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup intern_utf_conv
 */

#ifndef __UTFCONV_H__
#define __UTFCONV_H__

#include <cstdio>
#include <cstdlib>
#include <cwchar>

/**
 * Counts how many bytes is required for future utf-8 string using utf-16
 * \param string16: pointer to working utf-16 string
 * \return How many bytes must be allocated including NULL.
 */
size_t count_utf_8_from_16(const wchar_t *string16);

/**
 * Counts how many wchar_t (two byte) is required for future utf-16 string using utf-8
 * \param string8: pointer to working utf-8 string
 * \return How many bytes must be allocated including NULL.
 */
size_t count_utf_16_from_8(const char *string8);

/*
 * conv_utf_*** errors
 */

/** Error occurs when required parameter is missing. */
#define UTF_ERROR_NULL_IN (1 << 0)
/** Error if character is in illegal UTF range. */
#define UTF_ERROR_ILLCHAR (1 << 1)
/** Passed size is to small. It gives legal string with character missing at the end. */
#define UTF_ERROR_SMALL (1 << 2)
/** Error if sequence is broken and doesn't finish. */
#define UTF_ERROR_ILLSEQ (1 << 3)

/**
 * Converts utf-16 string to allocated utf-8 string
 * \param in16: utf-16 string to convert
 * \param out8: utf-8 string to string the conversion
 * \param size8: the allocated size in bytes of out8
 * \return Returns any errors occurred during conversion. See the block above,
 */
int conv_utf_16_to_8(const wchar_t *in16, char *out8, size_t size8);

/**
 * Converts utf-8 string to allocated utf-16 string
 * \param in8: utf-8 string to convert
 * \param out16: utf-16 string to string the conversion
 * \param size16: the allocated size in wchar_t (two byte) of out16
 * \return Returns any errors occurred during conversion. See the block above,
 */
int conv_utf_8_to_16(const char *in8, wchar_t *out16, size_t size16);

/**
 * Allocates and converts the utf-8 string from utf-16
 * \param in16: utf-16 string to convert
 * \param add: any additional size which will be allocated for new utf-8 string in bytes
 * \return New allocated and converted utf-8 string or NULL if in16 is 0.
 */
char *alloc_utf_8_from_16(const wchar_t *in16, size_t add);

/**
 * Allocates and converts the utf-16 string from utf-8
 * \param in8: utf-8 string to convert
 * \param add: any additional size which will be allocated for new utf-16 string
 * in wchar_t (two bytes)
 * \return New allocated and converted utf-16 string or NULL if in8 is 0.
 */
wchar_t *alloc_utf16_from_8(const char *in8, size_t add);

/* Easy allocation and conversion of new utf-16 string. New string has _16 suffix.
 * Must be deallocated with UTF16_UN_ENCODE in right order. */
#define UTF16_ENCODE(in8str) \
  if (1) { \
    wchar_t *in8str##_16 = alloc_utf16_from_8((const char *)in8str, 0)

#define UTF16_UN_ENCODE(in8str) \
  free(in8str##_16); \
  } \
  (void)0

#endif /* __UTFCONV_H__ */
