/*
 *  QTimeline.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
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
#include "QTimelineParam.h"
#include "QTimelineModule.h"
#include "QTimelineCueManager.h"
#include "QTimelineMenu.h"


#define QTIMELINE_SNAP      0.25f


class QTimeline
{
    
    friend class QTimelineCue;
    friend class QTimelineTrack;
    
    
public:

    struct ModuleCallbackArgs
    {
        std::string             name;
        std::string             type;
        float                   startTime;
        float                   duration;
        QTimelineTrackRef       trackRef;
    };
    
private:
    
    struct ModuleCallbacks
    {
        ci::CallbackMgr<void (ModuleCallbackArgs)>  createCb;
        ci::CallbackMgr<void (ModuleCallbackArgs)>  deleteCb;
    };
    
    std::map<std::string, ModuleCallbacks>  mModuleCallbacks;    
    
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
        
        mTracks.clear();
        
        mCueManager->clear();
    }
    
    void init();
    
    void update();
    
    void render();
    
    void play( bool play = true, PlayMode mode = FREE_RUN );
    
    void playCue();
    
    void addCue( std::string name, double startTime, double duration )
    {
        mCueManager->addCue( name, startTime, duration );
    }
    
    ci::Vec2f getTimeWindow() { return mTimeWindow; }
    
    bool isPlaying() { return mIsPlaying; }
    
    void addModule( QTimelineModule *module, float startAt, float duration );
    
    void addModule( QTimelineModule *module, float startAt, float duration, QTimelineTrackRef trackRef );

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
    void registerModule( std::string type, T *obj, void (T::*onCreateCallback)(ModuleCallbackArgs), void (T::*onDeleteCallback)(ModuleCallbackArgs) )
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
                ModuleCallbackArgs args = { moduleName, moduleType, startAt, duration, trackRef };
                it->second.createCb.call(args);
                return;
            }
    }
    
    
    void callDeleteModuleCb( std::string moduleName, std::string moduleType )
    {
        std::map<std::string, ModuleCallbacks>::iterator it;
        for ( it=mModuleCallbacks.begin(); it != mModuleCallbacks.end(); it++ )
            if( it->first == moduleType )
            {
                ModuleCallbackArgs args = { moduleName, moduleType };
                it->second.deleteCb.call(args);
                return;
            }
    }
    
    
    PlayMode getPlayMode() { return mPlayMode; }
    
    void toggleHelp() { mRenderHelp = !mRenderHelp; }
    
    void toggleDebugInfo() { mRenderDebug = !mRenderDebug; }
    
    float snapTime( float time )
    {
        time = (int)( time / QTIMELINE_SNAP ) * QTIMELINE_SNAP;
        return time;
    }
    
    void collapse()
    {
        for( size_t k=0; k < mTracks.size(); k++ )
            mTracks[k]->close();
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
    
    static ci::ColorA       mTransportBgCol;
    static ci::ColorA       mTransportTextCol;
    
    static ci::ColorA       mTracksBgCol;
    static ci::ColorA       mTracksBgOverCol;
    
    static ci::ColorA       mModulesBgCol;
    static ci::ColorA       mModulesBgOverCol;
    static ci::ColorA       mModulesTextCol;
    static ci::ColorA       mModulesHandleCol;
    static ci::ColorA       mModulesHandleOverCol;
    
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
    
private:
    
    
    ci::app::App            *mApp;
    ci::CallbackId          mCbMouseDown, mCbMouseDrag, mCbMouseUp, mCbMouseMove, mCbMouseWheel;
    ci::CallbackId          mCbResize;
    
    ci::TimelineRef         mTimeline;
    
    std::vector<QTimelineTrackRef>    mTracks;
    
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
    
    bool                    mIsMouseDragTimeBar;
    
    PlayMode                mPlayMode;

    QTimelineMenu           *mSelectedMenu;
    
    bool                    mRenderDebug;
    
    bool                    mRenderHelp;
    ci::gl::Texture         mHelpTex;
};


#endif
