/*
 *  QTimeline.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef QTIMELINE
#define QTIMELINE

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/Xml.h"
#include "cinder/Timeline.h"
#include "cinder/Easing.h"

#include "QTimelineTrack.h"
#include "QTimelineModuleItem.h"
#include "QTimelineAudioItem.h"
#include "QTimelineModule.h"
#include "QTimelineCueManager.h"
#include "QTimelineMenu.h"
#include "QTimelineOscController.h"

#define QTIMELINE_SNAP      0.25f

typedef std::shared_ptr<class QTimeline>		QTimelineRef;


class QTimeline : public std::enable_shared_from_this<QTimeline>
{
    friend class QTimelineCue;
    friend class QTimelineTrack;
    
public:

    struct CreateModuleCallbackArgs
    {
        std::string             name;
        std::string             type;
        float                   startTime;
        float                   duration;
        QTimelineTrackRef       trackRef;
    };
    
    struct DeleteModuleCallbackArgs
    {
        QTimelineItemRef  itemRef;
    };
    
private:
    
    struct ModuleCallbacks
    {
        ci::CallbackMgr<void (CreateModuleCallbackArgs)>  createCb;
        ci::CallbackMgr<void (DeleteModuleCallbackArgs)>  deleteCb;
    };
    
    std::map<std::string, ModuleCallbacks>  mModuleCallbacks;

    std::vector<QTimelineItemRef> mModulesMarkedForRemoval;
    
    
public:
    
    enum PlayMode
    {
        FREE_RUN,
        CUE_LIST
    };

    
    QTimeline() {}
    
    ~QTimeline()
    {
        mTracks.clear();
        
        delete mCueManager;
        mCueManager = NULL;
    }
    
    void clear()
    {
        closeMenu();
        
        mMouseOnTrack.reset();
        
        for( size_t k=0; k < mTracks.size(); k++ )
            mTracks[k]->clear();
        
        mTracks.clear();
        
        mCueManager->clear();
    }
    
    void init();
    
    void update();
    
    void render();
    
    void play( bool play = true, PlayMode mode = FREE_RUN );
    
    void playCue( int cueN = -1 );
    
    void addCue( std::string name, double startTime, double duration )
    {
        mCueManager->addCue( name, startTime, duration );
    }
    
    ci::Vec2f getTimeWindow() { return mTimeWindow; }
    
    bool isPlaying() { return mIsPlaying; }
    
    void addModule( QTimelineModuleRef moduleRef, float startAt, float duration );
    
    void addModule( QTimelineModuleRef moduleRef, float startAt, float duration, QTimelineTrackRef trackRef );
    
    void addTrack( QTimelineTrackRef ref ) { mTracks.push_back( ref ); }                    // just push back a new track ref
    
    void addTrack( QTimelineTrackRef newRef, QTimelineTrackRef otherRef, bool addAbove )    // add track in a specific position(relative to "otherRef")
    {
        for( auto trackIt = mTracks.begin(); trackIt != mTracks.end(); trackIt++ )
        {
            if ( trackIt->get() == otherRef.get() )
            {
                int offset = addAbove ? 0 : 1;
                mTracks.insert( trackIt + offset, newRef );
                update();
                return;
            }
        }
    }

    ci::TimelineRef getTimelineRef() { return mTimeline; }
    
    void toggleRender() { mIsVisible = !mIsVisible; }
    
    bool isVisible() { return mIsVisible; }
    
    std::vector<QTimelineTrackRef> getTracks() { return mTracks; }
    
    void loadTheme( const ci::fs::path &filepath );
    
    float getTimeFromPos( float x )
    {
        return mTimeWindow.x + ( mTimeWindow.y - mTimeWindow.x ) * ( x - mTransportRect.x1 ) / mTransportRect.getWidth();
    }
    
    float getPosFromTime( double time )
    {
        return mTransportRect.getWidth() * ( time - mTimeWindow.x ) / ( mTimeWindow.y - mTimeWindow.x );
    }

    float getTimeBarWidth() { return mTimeBarRect.x2 - mTimeBarRect.x1; }
    
    float getPxInSeconds( int n = 1 )
    {
        return n * ( mTimeWindow.y - mTimeWindow.x ) / mTransportRect.getWidth();
    }
    
    double getTime() { return mTimeline->getCurrentTime(); }
    
    void setTime( double time ) { mTimeline->stepTo( time ); }
    
    void updateCurrentTime();
    
    void save( const std::string &filename );
    
    void load( const std::string &filename );
    
    void openMenu( QTimelineMenu *menu, ci::Vec2f pos )
    {
        mSelectedMenu = menu;
        
        if ( mSelectedMenu )
            mSelectedMenu->open(pos);
    }

    void closeMenu( QTimelineMenu *menu = NULL )
    {
        if ( mSelectedMenu )
            mSelectedMenu->close();
        
        if ( mSelectedMenu == menu || menu == NULL )
            mSelectedMenu = NULL;
    }

    template<typename T>
    void registerModule( std::string type, T *obj, void (T::*onCreateCallback)(CreateModuleCallbackArgs), void (T::*onDeleteCallback)(DeleteModuleCallbackArgs) )
    {
        mModuleCallbacks[type].createCb.registerCb( std::bind1st( std::mem_fun( onCreateCallback ), obj ) );
        mModuleCallbacks[type].deleteCb.registerCb( std::bind1st( std::mem_fun( onDeleteCallback ), obj ) );
        
        for( size_t k=0; k < mTracks.size(); k++ )
            mTracks[k]->initMenu();
    }
    
    void callCreateModuleCb( std::string moduleName, std::string moduleType, float startAt, float duration, QTimelineTrackRef trackRef )
    {
        std::map<std::string, ModuleCallbacks>::iterator it;
        for ( it=mModuleCallbacks.begin(); it != mModuleCallbacks.end(); it++ )
            if( it->first == moduleType )
            {
                CreateModuleCallbackArgs args = { moduleName, moduleType, startAt, duration, trackRef };
                it->second.createCb.call(args);
                return;
            }
    }
    
    void callDeleteModuleCb( QTimelineItemRef itemRef )
    {
        std::string targetModuleType = ( (QTimelineModuleItem*)itemRef.get() )->getTargetType();

        std::map<std::string, ModuleCallbacks>::iterator it;
        for ( it=mModuleCallbacks.begin(); it != mModuleCallbacks.end(); it++ )
            if( it->first == targetModuleType )
            {
                DeleteModuleCallbackArgs args = { itemRef };
                it->second.deleteCb.call(args);
                return;
            }
    }

    void markModuleForRemoval( QTimelineItemRef item )
    {
        mModulesMarkedForRemoval.push_back( item );
    }
    
    PlayMode getPlayMode() { return mPlayMode; }
    
    void toggleHelp() { mRenderHelp = !mRenderHelp; }
    
    void toggleDebugInfo() { mRenderDebug = !mRenderDebug; }
    
    float snapTime( float time )
    {
        time = (int)( time / QTIMELINE_SNAP ) * QTIMELINE_SNAP;
        return time;
    }

    float snapPos( float pos )
    {
        float snap = getPosFromTime( QTIMELINE_SNAP );
        pos = (int)( pos / snap ) * snap;
        return pos;
    }
    
    void collapse()
    {
        for( size_t k=0; k < mTracks.size(); k++ )
            mTracks[k]->close();
    }
    
    void initOsc() // ( std::string host, int port )
    {
        mOscController.reset();
        mOscController = QTimelineOscControllerRef( new QTimelineOscController() );
    }
    
    void addOscMessage( std::string address, std::string args = "" )
    {
        if ( mOscController )
            mOscController->addMessage( address, args );
    }
    
    void step( int steps = 1 );

    void shutdown()
    {
        clear();
        
        thisRef = NULL;
    }
    
private:
    
    ci::ColorA getThemeColor( ci::XmlTree tree, std::string tag );
    
    void updateTimeWindow();
    
    void updateTime();
    
    void registerCallbacks();

    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    bool mouseWheel( ci::app::MouseEvent event );
    
    bool resize( ci::app::ResizeEvent event );
    
    void renderTimeBar();
    
    void renderDebugInfo();
    
    void eraseMarkedModules();

    
public:
    
    static std::function<float (float)> getEaseFnFromString( std::string fnStr )
    {
        std::function<float (float)> fn = ci::EaseNone();
        
        if( fnStr == "EaseInQuad" )
            fn = ci::EaseInQuad();
        
        else if( fnStr == "EaseOutQuad" )
            fn = ci::EaseOutQuad();
        
        else if( fnStr == "EaseInOutQuad" )
            fn = ci::EaseInOutQuad();
        
        else if( fnStr == "EaseOutInQuad" )
            fn = ci::EaseOutInQuad();
        
        else if( fnStr == "EaseStep" )
            fn = ci::EaseStep();
        
        else if( fnStr == "EaseNone" )
            fn = ci::EaseNone();
        
        return fn;
    }
    
    
    static ci::ColorA       mTimeBarBgCol;
    static ci::ColorA       mTimeBarFgCol;
    static ci::ColorA       mTimeBarTextCol;
    static ci::ColorA       mTimeBarMouseBarCol;
    static ci::ColorA       mTimeBarModuleRangeCol;
    
    static ci::ColorA       mTransportBgCol;
    static ci::ColorA       mTransportTextCol;
    
    static ci::ColorA       mTracksBgCol;
    static ci::ColorA       mTracksBgOverCol;
    
    static ci::ColorA       mModuleItemBgCol;
    static ci::ColorA       mModuleItemBgOverCol;
    static ci::ColorA       mModuleItemTextCol;
    static ci::ColorA       mModuleItemHandleCol;
    static ci::ColorA       mModuleItemHandleOverCol;
    
    static ci::ColorA       mAudioItemBgCol;
    static ci::ColorA       mAudioItemBgOverCol;
    static ci::ColorA       mAudioItemTextCol;
    static ci::ColorA       mAudioItemHandleCol;
    static ci::ColorA       mAudioItemHandleOverCol;
    
    static ci::ColorA       mParamsBgCol;
    static ci::ColorA       mParamsBgOverCol;
    static ci::ColorA       mParamsTextCol;
    
    static ci::ColorA       mKeyframesBgCol;
    static ci::ColorA       mKeyframesBgOverCol;
    static ci::ColorA       mKeyframesBgSelectedCol;
    static ci::ColorA       mKeyframesGraphCol;
    
    static ci::ColorA       mCueListBgCol;
    static ci::ColorA       mCueBgCol;
    static ci::ColorA       mCueBgOverCol;
    static ci::ColorA       mCueFgCol;
    static ci::ColorA       mCueTextCol;
    static ci::ColorA       mCueHandleCol;
    static ci::ColorA       mCueHandleOverCol;
    
public:
    
    static QTimeline*       getRef() { return thisRef; }
    
private:

    static  QTimeline       *thisRef;
    
private:
    
    
    ci::app::App            *mApp;
    ci::CallbackId          mCbMouseDown, mCbMouseDrag, mCbMouseUp, mCbMouseMove, mCbMouseWheel;
    ci::CallbackId          mCbResize;
    
    ci::Vec2f               mMousePrevPos;
    
    ci::TimelineRef         mTimeline;
    
    std::vector<QTimelineTrackRef>    mTracks;
    QTimelineTrackRef       mMouseOnTrack;
    
    QTimelineCueManager     *mCueManager;

    ci::Rectf               mWidgetRect;
    ci::Rectf               mTimeBarRect;
    ci::Rectf               mTransportRect;
    ci::Rectf               mTracksRect;
    ci::Rectf               mLabelsRect;
    
    bool                    mIsPlaying;
    
    double                  mPrevElapsedSeconds;        // prev time
    float                   mTimeNorm;                  // current time normalised
    
    ci::Vec2f               mTimeWindow;
    std::string             mTimeStr;
    
    ci::gl::TextureFontRef  mFontSmall;
    ci::gl::TextureFontRef  mFontMedium;
    ci::gl::TextureFontRef  mFontBig;
    
    float                   mZoom;
  
    bool                    mIsVisible;
    
    bool                    mMouseDragTimeBar;
    bool                    mMouseOnTimeBar;
    
    PlayMode                mPlayMode;

    QTimelineMenu           *mSelectedMenu;
    
    bool                    mRenderDebug;
    
    bool                    mRenderHelp;
    ci::gl::Texture         mHelpTex;
    
    QTimelineOscControllerRef   mOscController;
   
    
};


#endif
