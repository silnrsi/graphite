// SPDX-License-Identifier: MIT OR MPL-2.0 OR LGPL-2.1-or-later OR GPL-2.0-or-later
// Copyright (C) 2012 SIL International
using System;

namespace NGraphite
{
	public class Featureval : IDisposable
	{
		IntPtr _featureval;
		
		internal Featureval(IntPtr featureval)
		{
			_featureval = featureval;
		}
		
		internal IntPtr FeatureValPtr
		{
			get { return _featureval; }	
		}
		
		public Featureval Clone()
		{
			return new Featureval(Graphite2Api.FeaturevalClone(_featureval));
		}

		#region IDisposable implementation
		public void Dispose()
		{
			Graphite2Api.FeatureValDestroy(_featureval);
		}
		#endregion
	}
}

