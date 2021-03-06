// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_color.h"

#include <vector>

#include "fxjs/JS_Define.h"
#include "fxjs/cjs_event_context.h"
#include "fxjs/cjs_eventhandler.h"
#include "fxjs/cjs_object.h"
#include "fxjs/cjs_runtime.h"

const JSPropertySpec CJS_Color::PropertySpecs[] = {
    {"black", get_black_static, set_black_static},
    {"blue", get_blue_static, set_blue_static},
    {"cyan", get_cyan_static, set_cyan_static},
    {"dkGray", get_dark_gray_static, set_dark_gray_static},
    {"gray", get_gray_static, set_gray_static},
    {"green", get_green_static, set_green_static},
    {"ltGray", get_light_gray_static, set_light_gray_static},
    {"magenta", get_magenta_static, set_magenta_static},
    {"red", get_red_static, set_red_static},
    {"transparent", get_transparent_static, set_transparent_static},
    {"white", get_white_static, set_white_static},
    {"yellow", get_yellow_static, set_yellow_static},
    {0, 0, 0}};

const JSMethodSpec CJS_Color::MethodSpecs[] = {{"convert", convert_static},
                                               {"equal", equal_static},
                                               {0, 0}};

int CJS_Color::ObjDefnID = -1;

// static
void CJS_Color::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID = pEngine->DefineObj("color", FXJSOBJTYPE_STATIC,
                                 JSConstructor<CJS_Color, color>,
                                 JSDestructor<CJS_Color>);
  DefineProps(pEngine, ObjDefnID, PropertySpecs);
  DefineMethods(pEngine, ObjDefnID, MethodSpecs);
}

// static
v8::Local<v8::Array> color::ConvertPWLColorToArray(CJS_Runtime* pRuntime,
                                                   const CFX_Color& color) {
  v8::Local<v8::Array> array;
  switch (color.nColorType) {
    case CFX_Color::kTransparent:
      array = pRuntime->NewArray();
      pRuntime->PutArrayElement(array, 0, pRuntime->NewString(L"T"));
      break;
    case CFX_Color::kGray:
      array = pRuntime->NewArray();
      pRuntime->PutArrayElement(array, 0, pRuntime->NewString(L"G"));
      pRuntime->PutArrayElement(array, 1, pRuntime->NewNumber(color.fColor1));
      break;
    case CFX_Color::kRGB:
      array = pRuntime->NewArray();
      pRuntime->PutArrayElement(array, 0, pRuntime->NewString(L"RGB"));
      pRuntime->PutArrayElement(array, 1, pRuntime->NewNumber(color.fColor1));
      pRuntime->PutArrayElement(array, 2, pRuntime->NewNumber(color.fColor2));
      pRuntime->PutArrayElement(array, 3, pRuntime->NewNumber(color.fColor3));
      break;
    case CFX_Color::kCMYK:
      array = pRuntime->NewArray();
      pRuntime->PutArrayElement(array, 0, pRuntime->NewString(L"CMYK"));
      pRuntime->PutArrayElement(array, 1, pRuntime->NewNumber(color.fColor1));
      pRuntime->PutArrayElement(array, 2, pRuntime->NewNumber(color.fColor2));
      pRuntime->PutArrayElement(array, 3, pRuntime->NewNumber(color.fColor3));
      pRuntime->PutArrayElement(array, 4, pRuntime->NewNumber(color.fColor4));
      break;
  }
  return array;
}

// static
CFX_Color color::ConvertArrayToPWLColor(CJS_Runtime* pRuntime,
                                        v8::Local<v8::Array> array) {
  int nArrayLen = pRuntime->GetArrayLength(array);
  if (nArrayLen < 1)
    return CFX_Color();

  WideString sSpace =
      pRuntime->ToWideString(pRuntime->GetArrayElement(array, 0));
  if (sSpace == L"T")
    return CFX_Color(CFX_Color::kTransparent);

  float d1 = 0;
  if (nArrayLen > 1) {
    d1 = static_cast<float>(
        pRuntime->ToDouble(pRuntime->GetArrayElement(array, 1)));
  }

  if (sSpace == L"G")
    return CFX_Color(CFX_Color::kGray, d1);

  float d2 = 0;
  float d3 = 0;
  if (nArrayLen > 2) {
    d2 = static_cast<float>(
        pRuntime->ToDouble(pRuntime->GetArrayElement(array, 2)));
  }
  if (nArrayLen > 3) {
    d3 = static_cast<float>(
        pRuntime->ToDouble(pRuntime->GetArrayElement(array, 3)));
  }

  if (sSpace == L"RGB")
    return CFX_Color(CFX_Color::kRGB, d1, d2, d3);

  float d4 = 0;
  if (nArrayLen > 4) {
    d4 = static_cast<float>(
        pRuntime->ToDouble(pRuntime->GetArrayElement(array, 4)));
  }
  if (sSpace == L"CMYK")
    return CFX_Color(CFX_Color::kCMYK, d1, d2, d3, d4);

  return CFX_Color();
}

color::color(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject) {
  m_crTransparent = CFX_Color(CFX_Color::kTransparent);
  m_crBlack = CFX_Color(CFX_Color::kGray, 0);
  m_crWhite = CFX_Color(CFX_Color::kGray, 1);
  m_crRed = CFX_Color(CFX_Color::kRGB, 1, 0, 0);
  m_crGreen = CFX_Color(CFX_Color::kRGB, 0, 1, 0);
  m_crBlue = CFX_Color(CFX_Color::kRGB, 0, 0, 1);
  m_crCyan = CFX_Color(CFX_Color::kCMYK, 1, 0, 0, 0);
  m_crMagenta = CFX_Color(CFX_Color::kCMYK, 0, 1, 0, 0);
  m_crYellow = CFX_Color(CFX_Color::kCMYK, 0, 0, 1, 0);
  m_crDKGray = CFX_Color(CFX_Color::kGray, 0.25);
  m_crGray = CFX_Color(CFX_Color::kGray, 0.5);
  m_crLTGray = CFX_Color(CFX_Color::kGray, 0.75);
}

color::~color() {}

CJS_Return color::get_transparent(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crTransparent);
}

CJS_Return color::set_transparent(CJS_Runtime* pRuntime,
                                  v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crTransparent);
}

CJS_Return color::get_black(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crBlack);
}

CJS_Return color::set_black(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crBlack);
}

CJS_Return color::get_white(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crWhite);
}

CJS_Return color::set_white(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crWhite);
}

CJS_Return color::get_red(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crRed);
}

CJS_Return color::set_red(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crRed);
}

CJS_Return color::get_green(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crGreen);
}

CJS_Return color::set_green(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crGreen);
}

CJS_Return color::get_blue(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crBlue);
}

CJS_Return color::set_blue(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crBlue);
}

CJS_Return color::get_cyan(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crCyan);
}

CJS_Return color::set_cyan(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crCyan);
}

CJS_Return color::get_magenta(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crMagenta);
}

CJS_Return color::set_magenta(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crMagenta);
}

CJS_Return color::get_yellow(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crYellow);
}

CJS_Return color::set_yellow(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crYellow);
}

CJS_Return color::get_dark_gray(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crDKGray);
}

CJS_Return color::set_dark_gray(CJS_Runtime* pRuntime,
                                v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crDKGray);
}

CJS_Return color::get_gray(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crGray);
}

CJS_Return color::set_gray(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crGray);
}

CJS_Return color::get_light_gray(CJS_Runtime* pRuntime) {
  return GetPropertyHelper(pRuntime, &m_crLTGray);
}

CJS_Return color::set_light_gray(CJS_Runtime* pRuntime,
                                 v8::Local<v8::Value> vp) {
  return SetPropertyHelper(pRuntime, vp, &m_crLTGray);
}

CJS_Return color::GetPropertyHelper(CJS_Runtime* pRuntime, CFX_Color* var) {
  v8::Local<v8::Value> array = ConvertPWLColorToArray(pRuntime, *var);
  if (array.IsEmpty())
    return CJS_Return(pRuntime->NewArray());
  return CJS_Return(array);
}

CJS_Return color::SetPropertyHelper(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp,
                                    CFX_Color* var) {
  if (vp.IsEmpty() || !vp->IsArray())
    return CJS_Return(false);

  *var = ConvertArrayToPWLColor(pRuntime, pRuntime->ToArray(vp));
  return CJS_Return(true);
}

CJS_Return color::convert(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params) {
  int iSize = params.size();
  if (iSize < 2)
    return CJS_Return(false);
  if (params[0].IsEmpty() || !params[0]->IsArray())
    return CJS_Return(false);

  WideString sDestSpace = pRuntime->ToWideString(params[1]);
  int nColorType = CFX_Color::kTransparent;
  if (sDestSpace == L"T")
    nColorType = CFX_Color::kTransparent;
  else if (sDestSpace == L"G")
    nColorType = CFX_Color::kGray;
  else if (sDestSpace == L"RGB")
    nColorType = CFX_Color::kRGB;
  else if (sDestSpace == L"CMYK")
    nColorType = CFX_Color::kCMYK;

  CFX_Color color =
      ConvertArrayToPWLColor(pRuntime, pRuntime->ToArray(params[0]));

  v8::Local<v8::Value> array =
      ConvertPWLColorToArray(pRuntime, color.ConvertColorType(nColorType));
  if (array.IsEmpty())
    return CJS_Return(pRuntime->NewArray());
  return CJS_Return(array);
}

CJS_Return color::equal(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() < 2)
    return CJS_Return(false);
  if (params[0].IsEmpty() || !params[0]->IsArray() || params[1].IsEmpty() ||
      !params[1]->IsArray()) {
    return CJS_Return(false);
  }

  CFX_Color color1 =
      ConvertArrayToPWLColor(pRuntime, pRuntime->ToArray(params[0]));
  CFX_Color color2 =
      ConvertArrayToPWLColor(pRuntime, pRuntime->ToArray(params[1]));

  color1 = color1.ConvertColorType(color2.nColorType);
  return CJS_Return(pRuntime->NewBoolean(color1 == color2));
}
