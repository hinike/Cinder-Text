#include "text/SystemFonts.h"

#if defined( CINDER_MSW_DESKTOP )
#include <windows.h>
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#include <gdiplus.h>
#undef min
#undef max
#include "cinder/msw/CinderMsw.h"
#include "cinder/msw/CinderMswGdiPlus.h"
#pragma comment(lib, "gdiplus")
#endif

#include "cinder/Unicode.h"
#include "cinder/app/App.h"

#include <unordered_map>
#include <strsafe.h>


namespace txt
{
	HDC mFontDC = nullptr;

	SystemFonts::SystemFonts()
	{
		mFontDC = ::CreateCompatibleDC( NULL );

		loadFaces();

		for( auto& family : mFaces ) {
			ci::app::console() << family.first << std::endl;
			ci::app::console() << "---------------------" << std::endl;

			for( auto& style : family.second ) {
				ci::app::console() <<  style << std::endl;
			}

			ci::app::console() << std::endl;
		}

		getFont( "Arialz", "Bold" );

	}

#if defined( CINDER_MSW_DESKTOP )
	int CALLBACK EnumFacesExProc( ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam )
	{
		std::string familyName = ci::toUtf8( ( char16_t* )lpelfe->elfLogFont.lfFaceName );
		std::string style = ci::toUtf8( ( char16_t* )lpelfe->elfStyle );

		( *reinterpret_cast<std::map<std::string, std::vector<std::string>>*>( lParam ) )[familyName].push_back( style );

		return 1;
	}



	struct HFontRequest {
		LOGFONT logfont;
		std::string family;
		std::string style;
		bool fontFound = false;
	};

	HFontRequest mHFontRequest;

	int CALLBACK LoadHFontExProc( ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam )
	{
		HFontRequest* request = reinterpret_cast<HFontRequest*>( lParam );

		std::string familyName = ci::toUtf8( ( char16_t* )lpelfe->elfLogFont.lfFaceName );
		std::string style = ci::toUtf8( ( char16_t* )lpelfe->elfStyle );

		if( familyName == request->family && style == request->style ) {
			request->logfont = lpelfe->elfLogFont;
			request->fontFound = true;
		}

		return 1;
	}


	void SystemFonts::loadFaces()
	{
		mFaces.clear();

		::LOGFONT lf;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfFaceName[0] = '\0';

		std::u16string faceName = ci::toUtf16( "Arial" );
		::StringCchCopy( lf.lfFaceName, LF_FACESIZE, ( LPCTSTR )faceName.c_str() );

		::EnumFontFamiliesEx( mFontDC, &lf, ( FONTENUMPROC )EnumFacesExProc, reinterpret_cast<LPARAM>( &mFaces ), 0 );
	}

	void SystemFonts::getFont( std::string family, std::string style )
	{
		::LOGFONT lf;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfFaceName[0] = '\0';

		std::u16string faceName = ci::toUtf16( family );
		::StringCchCopy( lf.lfFaceName, LF_FACESIZE, ( LPCTSTR )faceName.c_str() );

		mHFontRequest.family = family;
		mHFontRequest.style = style;

		::EnumFontFamiliesEx( mFontDC, &lf, ( FONTENUMPROC )LoadHFontExProc, reinterpret_cast<LPARAM>( &mHFontRequest ), 0 );

		HFONT hFont;

		if( mHFontRequest.fontFound ) {
			hFont = ::CreateFontIndirect( &mHFontRequest.logfont );
			::SelectObject( mFontDC, hFont );

			void* buffer;
			::GetFontData( mFontDC, 0, 0, buffer, 0 );

		}
	}

#endif
}