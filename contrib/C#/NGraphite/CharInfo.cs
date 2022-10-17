// SPDX-License-Identifier: MIT OR MPL-2.0 OR LGPL-2.1-or-later OR GPL-2.0-or-later
// Copyright (C) 2012 SIL International
using System;

namespace NGraphite
{
	public class CharInfo
	{
		IntPtr _charInfo;
		
		internal CharInfo(IntPtr charInfoPtr)
		{
			_charInfo = charInfoPtr;
		}
		
		public UInt32 UnicodeChar
		{
			get { return Graphite2Api.CinfoUnicodeChar(_charInfo); }
		}
		
		public int BreakWeight
		{
			get { return Graphite2Api.CinfoBreakWeight(_charInfo); }	
		}
		
		public int After
		{
			get { return Graphite2Api.CinfoAfter(_charInfo); }
		}
		
		public int Before
		{
			get { return Graphite2Api.CinfoBefore(_charInfo); }
		}
		
		public int Base
		{
			get { return Graphite2Api.CinfoBase(_charInfo); }
		}
	}
}

