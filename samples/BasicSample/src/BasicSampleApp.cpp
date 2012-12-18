#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "Resources.h"
#include "QTimeline.h"


#include "BasicModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();
    
    void createModuleCallback( QTimeline::ModuleCallbackArgs args );
    void deleteModuleCallback( QTimeline::ModuleCallbackArgs args );
    
    QTimeline                   mTimeline;
    vector<QTimelineModule*>    mModules;
    
    float   mFps;

    gl::Texture                 mLogoTex;
    
    
};

void BasicSampleApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 800 );
}

void BasicSampleApp::setup()
{
    mTimeline.init();
    
    
    // register modules
    mTimeline.registerModule( "BasicModule", this, &BasicSampleApp::createModuleCallback, &BasicSampleApp::deleteModuleCallback );

    QTimelineModule *mod;
    mod = new BasicModule( "Sample module one" );
    mTimeline.addModule( mod, 2.0f, 12.0f );
    mod->init();
    mModules.push_back( mod );

    mod = new BasicModule( "Sample module two" );
    mTimeline.addModule( mod, 2.0f, 12.0f );
    mod->init();
    mModules.push_back( mod );
    
    mTimeline.addCue( "Cue 1", 0.0f, 3.0f );
    mTimeline.addCue( "Another cue", 4.0f, 5.0f );
    
    mTimeline.loadTheme( getAssetPath("theme1.xml").generic_string() );
    
    mLogoTex        = gl::Texture( loadImage( loadResource(RES_LOGO) ) );
}

void BasicSampleApp::keyDown( KeyEvent event )
{
    char    c       = event.getChar();
    int     code    = event.getChar();
    
    if ( event.isAltDown() )
    {
        if ( c == 'h' )
            mTimeline.toggleRender();
        
        else if ( c == 'f' )
            setFullScreen( !isFullScreen() );
    }
    
    
    if ( code == KeyEvent::KEY_SPACE )
        mTimeline.play( !mTimeline.isPlaying(), QTimeline::FREE_RUN );
    
    else if ( code == KeyEvent::KEY_RETURN )
        mTimeline.playCue();
    
    else if ( code == KeyEvent::KEY_DELETE )
        mTimeline.setTime(0.0f);
    
    else if ( code == KeyEvent::KEY_ESCAPE )
        mTimeline.closeMenu();
    
    else if ( code == 63 )
        mTimeline.toggleHelp();
    
    else if ( c == 'd' )
        mTimeline.toggleDebugInfo();
}

void BasicSampleApp::update()
{
    mTimeline.update();
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::enableAlphaBlending();
    
    for( size_t k=0; k < mModules.size(); k ++ )
        if ( mModules[k]->isPlaying() )
            mModules[k]->render();
    
    mTimeline.render();

    gl::color( Color::white() );
    
   gl::draw( mLogoTex, Vec2i( getWindowWidth() - mLogoTex.getWidth(), 5 ) );
}


void BasicSampleApp::createModuleCallback( QTimeline::ModuleCallbackArgs args )
{
    QTimelineModule *mod = NULL;
    
    if( args.name == "BasicModule" )
        mod = new BasicModule( "Untitled" );
    
    // ...
    
    if ( !mod )
        return;
    
    mTimeline.addModule( mod, args.startTime, args.duration, args.trackRef );
    mod->init();
    
    mModules.push_back( mod );
}


void BasicSampleApp::deleteModuleCallback( QTimeline::ModuleCallbackArgs args )
{
    for( size_t k=0; k < mModules.size(); k ++ )
        if ( mModules[k]->getName() == args.name )
        {
            delete mModules[k];
            mModules.erase( mModules.begin() + k );
            return;
        }
}


CINDER_APP_BASIC( BasicSampleApp, RendererGl )

