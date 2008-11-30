// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/**
* Modifications (c) 2008 Appcelerator, Inc
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/*
  JsClass class: 

  This is a patch of the original "CppBoundClass" in chromium that adds a
  function for converting into an NPObject* so it can be returned in the
  as value of the "result" object.

  This base class serves as a parent for C++ classes designed to be bound to
  JavaScript objects.

  Subclasses should define the constructor to build the property and method
  lists needed to bind this class to a JS object.  They should also declare
  and define member variables and methods to be exposed to JS through
  that object.

  See cpp_binding_example.{h|cc} for an example.
*/

#ifndef WEBKIT_GLUE_CPP_BOUNDCLASS_H__
#define WEBKIT_GLUE_CPP_BOUNDCLASS_H__

#include <map>
#include <vector>

#include "webkit/glue/cpp_variant.h"

#include "base/task.h"

class WebFrame;

typedef std::vector<CppVariant> CppArgumentList;

// JsClass lets you map Javascript method calls and property accesses
// directly to C++ method calls and CppVariant* variable access.
class JsClass {
 public:
  // The constructor should call BindMethod, BindProperty, and
  // SetFallbackMethod as needed to set up the methods, properties, and
  // fallback method.
  JsClass() { }
  virtual ~JsClass();

  // Given a WebFrame, BindToJavascript builds the NPObject that will represent
  // the class and binds it to the frame's window under the given name.  This
  // should generally be called from the WebView delegate's
  // WindowObjectCleared(). A class so bound will be accessible to JavaScript
  // as window.<classname>. The owner of the CppBoundObject is responsible for
  // keeping the object around while the frame is alive, and for destroying it
  // afterwards.
  void BindToJavascript(WebFrame* frame, const std::wstring& classname);

  // The type of callbacks.
  typedef Callback2<const CppArgumentList&, CppVariant*>::Type Callback;

  // Used by a test.  Returns true if a method with name |name| exists,
  // regardless of whether a fallback is registered.
  bool IsMethodRegistered(std::string name);

  NPObject* ToNPObject();

 protected:
  // Bind the Javascript method called |name| to the C++ callback |callback|.
  void BindCallback(std::string name, Callback* callback);
  
  // Bind property getter/setter callbacks
  void BindPropertyCallbacks(std::string name, Callback *getter, Callback *setter);

  // A wrapper for BindCallback, to simplify the common case of binding a
  // method on the current object.  Though not verified here, |method|
  // must be a method of this CppBoundClass subclass.
  template<typename T>
  void BindMethod(std::string name,
      void (T::*method)(const CppArgumentList&, CppVariant*)) {
    Callback* callback =
        NewCallback<T, const CppArgumentList&, CppVariant*>(
            static_cast<T*>(this), method);
    BindCallback(name, callback);
  }

  // Bind the Javascript property called |name| to a CppVariant |prop|.
  void BindProperty(std::string name, CppVariant* prop);

  // Bind property getter/setter functions
  template<typename T>
  void BindProperty(std::string name,
      void (T::*getter_method)(const CppArgumentList&, CppVariant*),
	  void (T::*setter_method)(const CppArgumentList&, CppVariant*)) {

    Callback* getter_callback =
        NewCallback<T, const CppArgumentList&, CppVariant*>(
            static_cast<T*>(this), getter_method);

	Callback* setter_callback =
        NewCallback<T, const CppArgumentList&, CppVariant*>(
            static_cast<T*>(this), setter_method);

	BindPropertyCallbacks(name, getter_callback, setter_callback);
  }


  // Set the fallback callback, which is called when when a callback is
  // invoked that isn't bound.
  // If it is NULL (its default value), a JavaScript exception is thrown in
  // that case (as normally expected). If non NULL, the fallback method is
  // invoked and the script continues its execution.
  // Passing NULL for |callback| clears out any existing binding.
  // It is used for tests and should probably only be used in such cases
  // as it may cause unexpected behaviors (a JavaScript object with a
  // fallback always returns true when checked for a method's
  // existence).
  void BindFallbackCallback(Callback* fallback_callback) {
    fallback_callback_.reset(fallback_callback);
  }

  // A wrapper for BindFallbackCallback, to simplify the common case of
  // binding a method on the current object.  Though not verified here,
  // |method| must be a method of this CppBoundClass subclass.
  // Passing NULL for |method| clears out any existing binding.
  template<typename T>
  void BindFallbackMethod(
      void (T::*method)(const CppArgumentList&, CppVariant*)) {
    if (method) {
      Callback* callback =
          NewCallback<T, const CppArgumentList&, CppVariant*>(
              static_cast<T*>(this), method);
      BindFallbackCallback(callback);
    } else {
      BindFallbackCallback(NULL);
    }
  }

  JsClass* CppVariantToJsClass(const CppVariant &variant);

  // some static helper functions
  static NPVariant StringToNPVariant(std::string &string);
  static bool GetObjectProperty(const CppVariant &variant, std::string prop, NPVariant *result);
  static int GetIntProperty(const CppVariant &variant, std::string prop);
  static double GetDoubleProperty(const CppVariant &variant, std::string prop);
  static bool GetBoolProperty(const CppVariant &variant, std::string prop);
  static const char* GetStringProperty(const CppVariant &variant, std::string prop);

  // Some fields are protected because some tests depend on accessing them,
  // but otherwise they should be considered private.
 
  typedef std::map<NPIdentifier, CppVariant*> PropertyList;
  typedef std::map<NPIdentifier, Callback*> MethodList;
  // These maps associate names with property and method pointers to be
  // exposed to JavaScript.
  PropertyList properties_;
  MethodList property_getters_, property_setters_;
  MethodList methods_;

  // The callback gets invoked when a call is made to an nonexistent method.
  scoped_ptr<Callback> fallback_callback_;

 private:
  // NPObject callbacks.
  friend struct CppNPObject;
  bool HasMethod(NPIdentifier ident);
  bool Invoke(NPIdentifier ident, const NPVariant* args, size_t arg_count,
              NPVariant* result);
  bool HasProperty(NPIdentifier ident);
  bool GetProperty(NPIdentifier ident, NPVariant* result);
  bool SetProperty(NPIdentifier ident, const NPVariant* value);

  // A list of all NPObjects we created and bound in BindToJavascript(), so we
  // can clean them up when we're destroyed.
  typedef std::vector<NPObject*> BoundObjectList;
  BoundObjectList bound_objects_;

  DISALLOW_EVIL_CONSTRUCTORS(JsClass);
};

#endif  // CPP_BOUNDCLASS_H__
