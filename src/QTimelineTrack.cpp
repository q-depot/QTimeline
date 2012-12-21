/*
 *  QTimelineTrack.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/Timeline.h"
#include "QTimeline.h"
#include "QTimelineTrack.h"
#include "QTimelineModuleItem.h"
#include "QTimelineModule.h"
#include "QTimelineParam.h"

using namespace ci;
using namespace ci::app;
using namespace std;

bool sortModulesHelper( QTimelineModuleItemRef a, QTimelineModuleItemRef b ) { return ( a->getStartTime() < b->getStartTime() ); }


QTimelineTrack::QTimelineTrack(  QTimeline *timeline, string name, XmlTree node  ) : mQTimeline(timeline), QTimelineWidget( name )
{
    mBgColor            = QTimeline::mTracksBgCol;
    mBgOverColor        = QTimeline::mTracksBgCol;
    
    mIsMouseOnTrack     = false;
    mIsTrackOpen        = true;
    
    loadXmlNode( node );
    
    mMouseDownAt        = 0.0f;
    
    initMenu();
}


QTimelineTrack::~QTimelineTrack()
{
    if ( mMenu )
        mQTimeline->closeMenu( mMenu );
}


ci::Rectf QTimelineTrack::render( ci::Rectf rect, ci::Vec2f timeWindow, double currentTime )
{
    int maxParamsN  = 0;
    
    std::vector<QTimelineModuleItemRef>     modulesInWindow;
    QTimelineModuleItemRef                  module;
    
    ci::Rectf                               trackRect, moduleRect;

    // gett all modules in time window and calculate the max number of params
    for( size_t k=0; k < mModules.size(); k ++ )
        if ( mModules[k]->isInWindow( timeWindow ) )
        {
            modulesInWindow.push_back( mModules[k] );
            if ( mModules[k]->mParams.size() > maxParamsN )
                maxParamsN = mModules[k]->mParams.size();
        }
    
    // calculate module track rect based on the max number of params
    trackRect = Rectf( rect.getUpperLeft() - Vec2f( 0, TIMELINE_MODULE_HEIGHT ), rect.getUpperRight() );
    
    if ( mIsTrackOpen )
        trackRect.offset( - maxParamsN * ci::Vec2f( 0, TIMELINE_PARAM_HEIGHT + TIMELINE_WIDGET_PADDING ) );
    
    setRect( Rectf( trackRect.getUpperLeft(), rect.getUpperRight() ) );
    
    if ( mIsMouseOnTrack ) ci::gl::color( mBgOverColor ); else ci::gl::color( mBgColor );
    ci::gl::drawSolidRect( mRect );
    
    for( size_t k=0; k < modulesInWindow.size(); k ++ )
    {
        module          = modulesInWindow[k];

        // render module
        moduleRect = makeRect( trackRect, timeWindow, module->getStartTime(), module->getEndTime() );
        module->setRect( moduleRect );
        
        // module render only use the module rect without calculating any offset or returning another rect, it's a bit special :]
        QTimelineParamRef nullPtr;
        module->render( mMouseOnModule == module && mMouseOnModule->isMouseOnParam( nullPtr ) );
        
        ci::Rectf r = moduleRect;
        
        if ( mIsTrackOpen )
        {
            // render params
            for( size_t i=0; i < module->mParams.size(); i++ )
                r = module->mParams[i]->render( r );
            
            // render keyframes
            for( size_t i=0; i < module->mParams.size(); i++ )
                module->mParams[i]->renderKeyframes();
        }
        
        module->setWidgetRect( Rectf( moduleRect.getUpperLeft(), r.getLowerRight() ) );
    }
    
    return mRectPaddedHeight;
}


bool QTimelineTrack::mouseDown( ci::app::MouseEvent event )
{
    mSelectedModule = mMouseOnModule;
    mMouseDownPos   = event.getPos();
    
    if ( mSelectedModule )
    {
        if ( mSelectedModule->contains( event.getPos() ) && event.isLeftDown() && getElapsedSeconds() - mMouseDownAt < 0.5f )
        {
            toggle();
            mMouseOnModule.reset();
            mSelectedModule.reset();
            return true;
        }
        
        mMouseDownAt = getElapsedSeconds();
        
        mSelectedModule->mouseDown( event );
        return true;
    }
    
    else if ( mRect.contains( mMouseDownPos ) )
    {
        if ( event.isRightDown() )
            mQTimeline->openMenu( mMenu, event.getPos() );
        
        else if ( event.isLeftDown() && mMenu->isVisible() )
            mQTimeline->closeMenu( mMenu );
    }
    
    
    return false;
}


bool QTimelineTrack::mouseUp( ci::app::MouseEvent event )
{
    if ( mSelectedModule )
        mSelectedModule->mouseUp( event );
    
    mSelectedModule.reset();
    
    mouseMove(event);
    
    return false;
}


bool QTimelineTrack::mouseMove( ci::app::MouseEvent event )
{
    mIsMouseOnTrack     = false;
    
    if ( mMouseOnModule )
        mMouseOnModule->mouseMove( event );
    
    mMouseOnModule.reset();
    
    if ( contains( event.getPos() ) )
    {
        mIsMouseOnTrack = true;

        for( size_t k=0; k < mModules.size(); k++ )
            if ( mModules[k]->mouseMove( event ) )
            {
                mMouseOnModule = mModules[k];
                break;
            }
    }

    mMousePrevPos = event.getPos();
    
    return false;
}


bool QTimelineTrack::mouseDrag( ci::app::MouseEvent event )
{
    if ( mSelectedModule )
        mSelectedModule->mouseDrag( event );
    
    return false;
}


Vec2f QTimelineTrack::getTimeWindow()
{
    return mQTimeline->getTimeWindow();
}


void QTimelineTrack::addModule( QTimelineModule *module, float startAt, float duration )
{
    TimelineRef timelineRef = mQTimeline->getTimelineRef();
    
    QTimelineModuleItemRef moduleItemRef = QTimelineModuleItem::create( module, startAt, duration, QTimelineTrackRef(this), timelineRef.get() );
    module->setItemRef( moduleItemRef );
    moduleItemRef->setStartTime( startAt );
    moduleItemRef->setDuration( duration );
    timelineRef->insert( moduleItemRef );
    
    mModules.push_back( moduleItemRef );
    
    sort( mModules.begin(), mModules.end(), sortModulesHelper );
}


//void QTimelineTrack::addModuleItem( QTimelineModuleItemRef moduleItemRef )
//{
//    mModules.push_back( moduleItemRef );
//    
//    sort( mModules.begin(), mModules.end(), sortModulesHelper );
//}


void QTimelineTrack::deleteModuleItem( QTimelineModuleItemRef moduleItemRef, bool removeFromTimeline )
{
    for( size_t k=0; k < mModules.size(); k++ )
        if ( mModules[k] == moduleItemRef )
        {
            if (removeFromTimeline)
                mQTimeline->mTimeline->remove( moduleItemRef );
            mModules.erase( mModules.begin()+k );
            return;
        }
    mSelectedModule.reset();
}


XmlTree QTimelineTrack::getXmlNode()
{
    XmlTree node( "track", "" );
    node.setAttribute( "name", mName );
    
    for( size_t k=0; k < mModules.size(); k++ )
        node.push_back( mModules[k]->getXmlNode() );
    
    return node;
}


void QTimelineTrack::loadXmlNode( ci::XmlTree node )
{

}


void QTimelineTrack::menuEventHandler( QTimelineMenuItem* item )
{
    if ( item->getMeta() == "create" )
    {
        mQTimeline->callCreateModuleCb( item->getName(), mMouseDownPos, getRef() );
        mQTimeline->closeMenu( mMenu );
    }
  
    else if (( item->getMeta() == "new_track_above") || (item->getMeta() == "new_track_below"))
    {
        mQTimeline->closeMenu( mMenu );

        QTimelineTrackRef ref( new QTimelineTrack( mQTimeline, "track untitled" ) );
        // std::find with shared_ptr wasn't happy here..
        for (auto i = mQTimeline->mTracks.begin(); i != mQTimeline->mTracks.end(); i++)
        {
            if (i->get() == this)
            {
                int offset = item->getMeta() == "new_track_above" ? 0 : 1;
                mQTimeline->mTracks.insert(i+offset, ref);
                mQTimeline->update();
                return;
            }
        }
        mQTimeline->mTracks.push_back(ref);
        mQTimeline->update();
    }
}


void QTimelineTrack::initMenu()
{
    mMenu->init( "TRACK MENU" );
    
    mMenu->addItem("New track above", "new_track_above", this, &QTimelineTrack::menuEventHandler);
    mMenu->addItem("New track below", "new_track_below", this, &QTimelineTrack::menuEventHandler);
    
    map<string, QTimeline::ModuleCallbacks>::iterator it;
    for ( it=mQTimeline->mModuleCallbacks.begin() ; it != mQTimeline->mModuleCallbacks.end(); it++ )
        mMenu->addItem( it->first, "create", this, &QTimelineTrack::menuEventHandler );
}

