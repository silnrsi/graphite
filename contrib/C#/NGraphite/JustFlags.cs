// SPDX-License-Identifier: MIT OR MPL-2.0 OR LGPL-2.1-or-later OR GPL-2.0-or-later
// Copyright (C) 2012 SIL International
using System;

namespace NGraphite
{
	public enum JustFlags
	{
		// Indicates that the start of the slot list is not at the start of a line
		JustStartInline = 1,
		// Indicates that the end of the slot list is not at the end of a line
		JustEndInline = 2
	}
}

