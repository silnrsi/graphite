using System;

namespace NGraphite
{
	public class Face : IDisposable
	{
		IntPtr _face;
		
		/// <summary>
		/// Initializes a new instance of the <see cref="NGraphite.Face"/> class.
		/// </summary>
		/// <param name='filename'>
		/// fullpath to a graphite enabled font.
		/// </param>
		public Face(string filename, FaceOptions options)
		{
			_face = Graphite2Api.MakeFileFace(filename, options);
		}
		
		public Face(string filename, uint segCacheMaxSize, FaceOptions options)
		{
			_face = Graphite2Api.MakeFileFaceWithSegCache(filename, segCacheMaxSize, options);
		}
		
		internal IntPtr FacePtr
		{
			get { return _face; }	
		}
		
		public Featureval FeaturevalForLang(string lang)
		{			
			IntPtr ptr = Graphite2Api.FaceFeaturevalForLang(_face, Graphite2Api.StrToTag(lang));
			if (ptr == IntPtr.Zero)
				return null;			                                                
			return new Featureval(ptr);
		}
		
		public FeatureRef FindFref(Int32 featid)
		{
			IntPtr ptr = Graphite2Api.FaceFindFref(_face, featid);
			if (ptr == IntPtr.Zero)
				return null;
			return new FeatureRef(ptr);
		}
		
		public UInt16 NFref()
		{
			return Graphite2Api.FaceNFref(_face);
		}
		
		public FeatureRef Fref(UInt16 i)
		{
			IntPtr ptr = Graphite2Api.FaceFref(_face, i);
			if (ptr == IntPtr.Zero)
				return null;			
			return new FeatureRef(ptr);
		}
		
		public ushort NLanguages()
		{
			return Graphite2Api.FaceNLanguages(_face);	
		}
		
		public UInt32 LangByIndex(UInt16 i)
		{
			return Graphite2Api.FaceLangByIndex(_face, i);
		}
		
		public short NGlyphs()
		{
			return Graphite2Api.FaceNGlyphs(_face);	
		}

		#region IDisposable implementation
		public void Dispose()
		{
			Graphite2Api.FaceDestroy(_face);
		}
		#endregion
	}
}

