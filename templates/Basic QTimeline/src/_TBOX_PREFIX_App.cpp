
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "QTimeline.h"
#include "BasicModule.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class _TBOX_PREFIX_App : public AppNative {
  public:
  	void prepareSettings( Settings *settings );
	void shutdown();
  	void setup();
  	void keyDown( KeyEvent event );
  	void update();
  	void draw();
    
      void createModuleCallback( QTimeline::CreateModuleCallbackArgs args );
      void deleteModuleCallback( QTimeline::DeleteModuleCallbackArgs args );
    
      QTimelineRef                    mTimeline;
      vector<QTimelineModuleRef>      mModules;
  };


  void _TBOX_PREFIX_App::prepareSettings( Settings *settings )
  {
      settings->setWindowSize( 1200, 800 );
  }


  void _TBOX_PREFIX_App::shutdown()
  {
      mTimeline->shutdown();
      mTimeline.reset();
  }


  void _TBOX_PREFIX_App::setup()
  {
      mTimeline = QTimelineRef( new QTimeline() );
    
      mTimeline->init();
    
     // mTimeline->initOsc();
    
      // register modules
      mTimeline->registerModule( "BasicModule", this, &_TBOX_PREFIX_App::createModuleCallback, &_TBOX_PREFIX_App::deleteModuleCallback );
    
      // create a timeline item
      QTimelineItemRef    item = mTimeline->addModule( 3.0f, 8.0f, "Fuffa" );

      // create a module
      QTimelineModuleRef  mod = QTimelineModuleRef( new BasicModule( item ) );
      mod->init();
    
      // set item target
      item->setTargetModule( mod );
    
      mModules.push_back( mod );
    
      // Cue list
      mTimeline->addCue( "Cue 1", 0.0f, 3.0f );
      mTimeline->addCue( "Another cue", 4.0f, 5.0f );
    
      mTimeline->loadTheme( getAssetPath( "default_theme.xml" ).generic_string() );
  }

  void _TBOX_PREFIX_App::keyDown( KeyEvent event )
  {
      char    c       = event.getChar();
      int     code    = event.getChar();
    
      if ( event.isAltDown() )
      {
          if ( c == 'f' )
              setFullScreen( !isFullScreen() );
        
          else if ( c == 's' )
              mTimeline->save( getAssetPath("") / "timeline.xml" );
        
          else if ( c == 'l' )
              mTimeline->load( getAssetPath("") / "timeline.xml" );
      }
    
      if ( code == KeyEvent::KEY_SPACE )
          mTimeline->play( !mTimeline->isPlaying(), QTimeline::FREE_RUN );
    
      else if ( code == KeyEvent::KEY_RETURN )
          mTimeline->playCue();
    
      else if ( code == KeyEvent::KEY_DELETE )
          mTimeline->setTime(0.0f);
    
      else if ( code == KeyEvent::KEY_ESCAPE )
          mTimeline->closeMenu();
    
      else if ( code == 63 )
          mTimeline->toggleHelp();
    
      else if ( c == 'd' )
          mTimeline->toggleDebugInfo();
    
      else if ( c == 'c' )
          mTimeline->collapse();

      else if ( (int)c >= 48 && (int)c <= 57 )
          mTimeline->playCue( (int)c - 49 );
    
      else if ( c == 'h' )
          mTimeline->toggleRender();
    
      else if ( code == 2)
          mTimeline->step(-);
    
      else if ( code == 3 )
          mTimeline->step(1);
    
      // else if ( code == 0 )
      //     mTimeline->stepTimeWindow(1);
      //     
      // else if ( code == 1 )
      //     mTimeline->stepTimeWindow(-1);
    
  }


  void _TBOX_PREFIX_App::update()
  {
      mTimeline->update();
  }


  void _TBOX_PREFIX_App::draw()
  {
      gl::clear( Color( 0, 0, 0 ) );
      gl::enableAlphaBlending();
  
      auto tracks = mTimeline->getTracks();
      for ( auto i = tracks.rbegin(); i != tracks.rend(); i++ )
          (*i)->renderActiveItem();
    
      mTimeline->render();

      gl::color( Color::white() );
  }


  void _TBOX_PREFIX_App::createModuleCallback( QTimeline::CreateModuleCallbackArgs args )
  {
      QTimelineModuleRef  mod;
      QTimelineItemRef    item = args.itemRef;
    
      if( args.type == "BasicModule" )
          mod = QTimelineModuleRef( new BasicModule( item ) );
    
      // else if ...
    
      if ( !mod )
          return;

      //
    
      item->setTargetModule( mod );
    
      mod->init();
    
      mModules.push_back( mod );
  }


  void _TBOX_PREFIX_App::deleteModuleCallback( QTimeline::DeleteModuleCallbackArgs args )
  {
      for( size_t k=0; k < mModules.size(); k++ )
          if ( mModules[k] == args.itemRef->getTargetModule() )
          {
              mModules.erase( mModules.begin() + k );
              return;
          }
  }


CINDER_APP_NATIVE( _TBOX_PREFIX_App, RendererGl )


