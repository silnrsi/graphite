using System;
using NUnit.Framework;
using NGraphite;

namespace NGraphiteTests
{
	[TestFixture()]
	public class FaceTests
	{
		[Test()]
		public void Face_ConstructPaduakFaceInstance_DoesNotThrowException()
		{
			new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default).Dispose();
		}
		
		[Test()]
		public void Face_ConstructPaduakFaceInstanceWithSegCache_DoesNotThrowException()
		{
			new Face(TestConstants.PaduakFontLocation, 100, FaceOptions.face_default).Dispose();
		}
		
		[Test()]
		public void FeaturevalForLang_EnLang_ReturnsNonNullFeatureval()
		{
			using (var face = new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default))
			{
				Featureval val = face.FeaturevalForLang("en");
				Assert.NotNull(val);					
			}
		}
		
		[Test()]
		public void FindFref_FindFreatureForkdotString_ReturnsNonNullFeatureRef()
		{
			using (var face = new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default))
			{
				FeatureRef featureRef = face.FindFref(1801744244);
				Assert.NotNull(featureRef);
			}
		}
		
		[Test()]
		public void NFref_PaduakFace_ReturnNineOrMoreFeatures()
		{
			using (var face = new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default))
			{
				Assert.GreaterOrEqual(face.NFref(), 9);
			}
		}
		
		[Test()]
		public void Fref_PaduakFace_ReturnNonNullFeatureRef()
		{
			using (var face = new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default))
			{
				FeatureRef featureRef = face.Fref(0);
				Assert.NotNull(featureRef);
			}
		}
		
		[Test()]
		public void NLanguages_PaduakFace_ReturnsExpectedNumberOfLanguages()
		{
			using (var face = new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default))
			{
				Assert.AreEqual(3, face.NLanguages());
			}
		}
		
		[Test()]
		public void LangByIndex_PaduakFaceIndexZero_ReturnsNonZeroLangId()
		{
			using (var face = new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default))
			{
				Assert.AreNotEqual(0, face.LangByIndex(0));	
			}
		
		}
		
		[Test()]
		public void NGlyphs_PaduakFace_ReturnsExpectedNumberOfGlyphs()
		{
			using (var face = new Face(TestConstants.PaduakFontLocation, FaceOptions.face_default))
			{
				Assert.AreEqual(445, face.NGlyphs());
			}
		}
	}
}

