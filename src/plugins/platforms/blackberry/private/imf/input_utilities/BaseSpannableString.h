/*
 * SpannableString.h
 *
 *  Created on: Mar 3, 2011
 *  Author: achapiro
 *  Research In Motion Ltd., 2010-
 *  295 Phillip Street Waterloo, ON N2L 3W8
 *  Canada
 *  This software is the confidential and proprietary information of
 *  Research In Motion Ltd. ("Confidential Information").  You shall not
 *  disclose such Confidential Information and shall use it only in
 *  accordance with the terms of the license agreement you entered into
 *  with Research In Motion.
 */

#ifndef BASE_SPANNABLESTRING_H_
#define BASE_SPANNABLESTRING_H_

#include <imf/spannable_string.h>

#ifdef __TEST__
#include <iostream>
#include <sstream>
#include <string>
#endif

namespace InputMethodSystem {

class BaseSpannableString  {
protected:
	spannable_string_t * _pSpannableString;
	bool _owner;
public:
	BaseSpannableString();
	BaseSpannableString(const BaseSpannableString& other);
	BaseSpannableString(spannable_string_t * pSpannableString, bool owner = false) ;
	BaseSpannableString(const imf_char_t * pText, size_t length, const span_t * pSpans, size_t nSpans);

	virtual ~BaseSpannableString();

	virtual BaseSpannableString& operator=(const BaseSpannableString& other);
public:
	imf_char_t const getCharacterAt(int32_t index);

	imf_char_t * getChars() const { return _pSpannableString->str; }

	int32_t getSpansCount() const { return _pSpannableString->spans_count; }

	span_t *getSpans(size_t start_pos, size_t end_pos, size_t* spans_count);

	size_t getLength() const { return _pSpannableString->length; }

	void setSpan(uint64_t attributes, size_t start_pos, size_t end_pos,
		bool overwrite = true);

	void setSpanAndText(const imf_char_t * pText, size_t start_pos, size_t length,
		size_t replLength, int64_t attributes, bool overwrite = true);

	void clear();

	spannable_string_t * getForC() const { return _pSpannableString; }

	spannable_string_t * cloneForC() const;

	static spannable_string_t * cloneForC(spannable_string_t * from);
	static void destroy(spannable_string_t * what);


#ifdef __TEST__
	const char * toChart(std::string& strto);
	static void toChart(span_t * pSpan, std::stringstream& sout);
	static const char * toChart(uint64_t attr, size_t len, std::string& strto);
	static void toChart(uint64_t attr, size_t len, std::stringstream& sout);
	const char * toString(std::string& strto);
#endif
protected:
	virtual void init();
	void checkCorrectness();
};

}

#endif /* BASE_SPANNABLESTRING_H_ */
