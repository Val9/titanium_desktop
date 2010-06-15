/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _UI_MODULE_H_
#define _UI_MODULE_H_

#include "ui.h"

#include "ui_binding.h"
#include "script_evaluator.h"

namespace ti {

	class UIModule : public kroll::Module
	{
		KROLL_MODULE_CLASS(UIModule)

		public:
		static UIModule* GetInstance() { return instance_; }
		AutoUIBinding GetUIBinding() { return uiBinding; }
		void Start();
		static bool IsResourceLocalFile(std::string string);

		protected:
		DISALLOW_EVIL_CONSTRUCTORS(UIModule);
		static UIModule* instance_;
		AutoUIBinding uiBinding;

	};
}

#endif
