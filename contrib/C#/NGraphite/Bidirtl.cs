// SPDX-License-Identifier: MIT OR MPL-2.0 OR LGPL-2.1-or-later OR GPL-2.0-or-later
// Copyright (C) 2012 SIL International
using System;

namespace NGraphite
{
	public enum Bidirtl
	{
		/// Underlying paragraph direction is RTL
		Rtl = 1,
		/// Set this to not run the bidi pass internally, even if the font asks for it.
		/// This presumes that the segment is in a single direction.
		Nobidi = 2,
		/// Disable auto mirroring for rtl text
		Nomirror = 4
	}
}
