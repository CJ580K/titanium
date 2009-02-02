/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ui_module.h"
#include <stdlib.h>

using namespace ti;

#define TI_SET_BOOL(method, index) \
{ \
	SharedValue arg = args.at(index);\
	if (arg->IsBool()) \
	{ \
		this->method(arg->ToBool()); \
	}\
	else if (arg->IsInt()) \
	{ \
		this->method(arg->ToInt()==1); \
	} \
	else if (arg->IsString()) \
	{ \
		std::string yn(arg->ToString()); \
		if (yn=="yes" || yn=="true" || yn=="1") \
		{ \
			this->method(true); \
		} \
		else \
		{ \
			this->method(false); \
		} \
	} \
}

std::vector<UserWindow *> UserWindow::windows;
std::map<UserWindow*, std::vector<UserWindow*> > UserWindow::windowsMap;

UserWindow::UserWindow(kroll::Host *host, WindowConfig *config) :
	kroll::StaticBoundObject(), parent(NULL)
{

	this->host = host;
	this->config = config;

	/* this object is accessed by Titanium.Window.currentWindow */
	this->SetMethod("hide", &UserWindow::_Hide);
	this->SetMethod("show", &UserWindow::_Show);
	this->SetMethod("isUsingChrome", &UserWindow::_IsUsingChrome);
	this->SetMethod("setUsingChrome", &UserWindow::_SetUsingChrome);
	this->SetMethod("isFullScreen", &UserWindow::_IsFullScreen);
	this->SetMethod("setFullScreen", &UserWindow::_SetFullScreen);
	this->SetMethod("getID", &UserWindow::_GetId);
	this->SetMethod("open", &UserWindow::_Open);
	this->SetMethod("close", &UserWindow::_Close);
	this->SetMethod("getX", &UserWindow::_GetX);
	this->SetMethod("setX", &UserWindow::_SetX);
	this->SetMethod("getY", &UserWindow::_GetY);
	this->SetMethod("setY", &UserWindow::_SetY);
	this->SetMethod("getWidth", &UserWindow::_GetWidth);
	this->SetMethod("setWidth", &UserWindow::_SetWidth);
	this->SetMethod("getMaxWidth", &UserWindow::_GetMaxWidth);
	this->SetMethod("setMaxWidth", &UserWindow::_SetMaxWidth);
	this->SetMethod("getMinWidth", &UserWindow::_GetMinWidth);
	this->SetMethod("setMinWidth", &UserWindow::_SetMinWidth);
	this->SetMethod("getHeight", &UserWindow::_GetHeight);
	this->SetMethod("setHeight", &UserWindow::_SetHeight);
	this->SetMethod("getMaxHeight", &UserWindow::_GetMaxHeight);
	this->SetMethod("setMaxHeight", &UserWindow::_SetMaxHeight);
	this->SetMethod("getMinHeight", &UserWindow::_GetMinHeight);
	this->SetMethod("setMinHeight", &UserWindow::_SetMinHeight);
	this->SetMethod("getBounds", &UserWindow::_GetBounds);
	this->SetMethod("setBounds", &UserWindow::_SetBounds);
	this->SetMethod("getTitle", &UserWindow::_GetTitle);
	this->SetMethod("setTitle", &UserWindow::_SetTitle);
	this->SetMethod("getURL", &UserWindow::_GetURL);
	this->SetMethod("setURL", &UserWindow::_SetURL);
	this->SetMethod("isResizable", &UserWindow::_IsResizable);
	this->SetMethod("setResizable", &UserWindow::_SetResizable);
	this->SetMethod("isMaximizable", &UserWindow::_IsMaximizable);
	this->SetMethod("setMaimizable", &UserWindow::_SetMaximizable);
	this->SetMethod("isMinimizable", &UserWindow::_IsMinimizable);
	this->SetMethod("setMinizable", &UserWindow::_SetMinimizable);
	this->SetMethod("isCloseable", &UserWindow::_IsCloseable);
	this->SetMethod("setCloseable", &UserWindow::_SetCloseable);
	this->SetMethod("isVisible", &UserWindow::_IsVisible);
	this->SetMethod("setVisible", &UserWindow::_SetVisible);
	this->SetMethod("getTransparency", &UserWindow::_GetTransparency);
	this->SetMethod("setTransparency", &UserWindow::_SetTransparency);
	this->SetMethod("setMenu", &UserWindow::_SetMenu);
	this->SetMethod("getMenu", &UserWindow::_GetMenu);
	this->SetMethod("setIcon", &UserWindow::_SetIcon);
	this->SetMethod("getIcon", &UserWindow::_GetIcon);

	this->SetMethod("createWindow", &UserWindow::_CreateWindow);
	this->SetMethod("getParent", &UserWindow::_GetParent);
}

void UserWindow::Open(UserWindow *window)
{
	// Don't do any refcounting here,
	// since we are holding onto our copy
	windows.push_back(window);
}

void UserWindow::Close(UserWindow *window)
{
	// check to see if we have a parent, and if so,
	// remove us from the parent list
	UserWindow *parent = window->GetParent();
	if (parent!=NULL)
	{
		UserWindow::RemoveChild(parent, window);
	}

	// see if we have any open child windows
	std::vector<UserWindow*> children = windowsMap[window];
	std::vector<UserWindow*>::iterator iter;
	for (iter = children.begin(); iter != children.end(); iter++)
	{
		UserWindow *child = (*iter);
		child->Close();
	}
	children.clear();
	windowsMap.erase(window);


	// delete from vector
	for (iter = windows.begin(); iter != windows.end(); iter++)
	{
		if ((*iter) == window)
		{
			break;
		}
	}
	if (iter != windows.end())
	{
		windows.erase(iter);
	}

	// when we have no more windows, we exit ...
	if (windows.size()==0)
	{
#if defined(OS_OSX)
		[NSApp terminate:nil];
#elif defined(OS_WIN32)
		ExitProcess(0);
#else
		//TODO: in Win32, do we exit some other way??
		exit(0);
#endif
	}
}


void UserWindow::_Hide(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Hide();
}

void UserWindow::_Show(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Show();
}

void UserWindow::_IsUsingChrome(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsUsingChrome());
}

void UserWindow::_SetUsingChrome(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetUsingChrome, 0);
}

void UserWindow::_IsUsingScrollbars(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsUsingScrollbars());
}

void UserWindow::_IsFullScreen(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsFullScreen());
}

void UserWindow::_SetFullScreen(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetFullScreen, 0);
}

void UserWindow::_GetId(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetId().c_str());
}

void UserWindow::_Open(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Open();
}

void UserWindow::_Close(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Close();
}

void UserWindow::_GetX(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetX());
}

void UserWindow::_SetX(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetX(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetY(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetY());
}

void UserWindow::_SetY(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetY(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetWidth());
}

void UserWindow::_SetWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMinWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMinWidth());
}

void UserWindow::_SetMinWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMinWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMaxWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMaxWidth());
}

void UserWindow::_SetMaxWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMaxWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetHeight());
}

void UserWindow::_SetHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMinHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMinHeight());
}

void UserWindow::_SetMinHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMinHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMaxHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMaxHeight());
}

void UserWindow::_SetMaxHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMaxHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetBounds(const kroll::ValueList& args, kroll::SharedValue result)
{
	Bounds bounds = this->GetBounds();
	kroll::StaticBoundObject *b = new kroll::StaticBoundObject();
	b->Set("x", kroll::Value::NewInt(bounds.x));
	b->Set("y", kroll::Value::NewInt(bounds.y));
	b->Set("width", kroll::Value::NewInt(bounds.width));
	b->Set("height", kroll::Value::NewInt(bounds.height));
	result->SetObject(b);
}

void UserWindow::_SetBounds(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0 && args.at(0)->IsObject())
	{
		Bounds bounds;
		kroll::BoundObject* o = args[0]->ToObject();

		kroll::Value* x = o->Get("x");
		kroll::Value* y = o->Get("y");
		kroll::Value* width = o->Get("width");
		kroll::Value* height = o->Get("height");

		bounds.x = x->ToInt();
		bounds.y = y->ToInt();
		bounds.width = width->ToInt();
		bounds.height = height->ToInt();

		this->SetBounds(bounds);
	}
}

void UserWindow::_GetTitle(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetTitle().c_str());
}

void UserWindow::_SetTitle(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		std::string title = args.at(0)->ToString();
		this->SetTitle(title);
	}
}

void UserWindow::_GetURL(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetURL().c_str());
}

void UserWindow::_SetURL(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		std::string url = args.at(0)->ToString();
		this->SetURL(url);
	}
}

void UserWindow::_IsResizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsResizable());
}

void UserWindow::_SetResizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetResizable, 0);
}

void UserWindow::_IsMaximizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsMaximizable());
}

void UserWindow::_SetMaximizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetMaximizable, 0);
}

void UserWindow::_IsMinimizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsMinimizable());
}

void UserWindow::_SetMinimizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetMinimizable, 0);
}

void UserWindow::_IsCloseable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsCloseable());
}

void UserWindow::_SetCloseable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetCloseable, 0);
}

void UserWindow::_IsVisible(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsVisible());
}

void UserWindow::_SetVisible(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetVisible, 0);
}

void UserWindow::_GetTransparency(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetTransparency());
}

void UserWindow::_SetTransparency(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetTransparency(args.at(0)->ToDouble());
	}
}

void UserWindow::_SetMenu(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedPtr<MenuItem> menu = NULL; // A NULL value is an unset
	if (args.size() > 0 && args.at(0)->IsList())
	{
		menu = args.at(0)->ToList().cast<MenuItem>();
	}
	this->SetMenu(menu);
}

void UserWindow::_GetMenu(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetObject(this->GetMenu());
}

void UserWindow::_SetIcon(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedString icon_path = NULL; // a NULL value is an unset
	if (args.size() > 0 && args.at(0)->IsString())
	{
		const char *icon_url = args.at(0)->ToString();
		icon_path = UIModule::GetResourcePath(icon_url);
	}
	this->SetIcon(icon_path);
}

void UserWindow::_GetIcon(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetIcon()->c_str());
}

void UserWindow::_GetParent(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (this->parent==NULL)
	{
		result->SetNull();
		return;
	}
	SharedBoundObject o = this->GetParent();
	result->SetObject(o);
}

void UserWindow::_CreateWindow(const ValueList& args, SharedValue result)
{
	SharedBoundObject props = SharedBoundObject(new StaticBoundObject());
	if (args.size() > 0 && args.at(0)->IsObject())
		props = args.at(0)->ToObject();

	result->SetObject(this->CreateWindow(props));
}

SharedBoundObject UserWindow::CreateWindow(SharedBoundObject properties)
{
	//TODO: wrap in sharedptr
	WindowConfig *config = new WindowConfig();
	config->UseProperties(properties);

	UserWindow* window = this->WindowFactory(this->host, config);

	// Track parent/child relationship.
	window->SetParent(this); 
	return window;

}

std::vector<UserWindow*>& UserWindow::GetWindows()
{
	return UserWindow::windows;
}

void UserWindow::SetParent(UserWindow *parent)
{
	this->parent = parent;
	parent->AddChild(this);
}

UserWindow* UserWindow::GetParent()
{
	return this->parent;
}

void UserWindow::AddChild(UserWindow *window)
{
	UserWindow::AddChild(this, window);
}

void UserWindow::RemoveChild(UserWindow *window)
{
	UserWindow::RemoveChild(this,window);
}

void UserWindow::AddChild(UserWindow *parent, UserWindow *child)
{
	std::vector<UserWindow*> children = windowsMap[parent];
	children.push_back(child);
	windowsMap[parent]=children;
}

void UserWindow::RemoveChild(UserWindow *parent, UserWindow *child)
{
	std::vector<UserWindow*> children = windowsMap[parent];
	std::vector<UserWindow*>::iterator iter;
	for (iter = children.begin(); iter != children.end(); iter++) {
		if ((*iter) == child) {
			break;
		}
	}
	if (iter != children.end()) {
		children.erase(iter);
	}
}
