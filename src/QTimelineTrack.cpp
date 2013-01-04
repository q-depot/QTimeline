/*
 *  QTimelineTrack.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/Timeline.h"
#include "QTimeline.h"
#include "QTimelineTrack.h"

using namespace ci;
using namespace ci::app;
using namespace std;

bool sortModulesHelper( QTimelineItemRef a, QTimelineItemRef b ) { return ( a->getStartTime() < b->getStartTime() ); }


QTimelineTrack::QTimelineTrack( string name ) : QTimelineWidget( name )
{
    mBgColor            = QTimeline::mTracksBgCol;
    mBgOverColor        = QTimeline::mTracksBgCol;
    
    mIsMouseOnTrack     = false;
    mIsTrackOpen        = false;
    
    mMouseDownAt        = 0.0f;
    
    initMenu();
}


QTimelineTrack::~QTimelineTrack()
{
    clear();
}


void QTimelineTrack::clear()
{
    if ( mMenu )
        QTimeline::getRef()->closeMenu( mMenu );
    
    for( size_t k=0; k < mModules.size(); k++ )
        markModuleForRemoval( mModules[k] );
    
    QTimeline::getRef()->eraseMarkedModules();
}


ci::Rectf QTimelineTrack::render( ci::Rectf rect, ci::Vec2f timeWindow, double currentTime )
{
    int maxParamsN  = 0;
    
    std::vector<QTimelineItemRef>   itemsInWindow;
    QTimelineItemRef                itemRef;
    
    ci::Rectf                       trackRect, itemRect;

    // gett all modules in time window and calculate the max number of params
    for( size_t k=0; k < mModules.size(); k ++ )
        if ( mModules[k]->isInWindow( timeWindow ) )
        {
            itemsInWindow.push_back( mModules[k] );
            if ( mModules[k]->getNumParams() > maxParamsN )
                maxParamsN = mModules[k]->getNumParams();
        }
    
    // calculate module track rect based on the max number of params
    trackRect = Rectf( rect.getUpperLeft() - Vec2f( 0, TIMELINE_MODULE_HEIGHT ), rect.getUpperRight() );
    
    if ( mIsTrackOpen )
        trackRect.offset( - maxParamsN * ci::Vec2f( 0, TIMELINE_PARAM_HEIGHT + TIMELINE_WIDGET_PADDING ) );
    
    setRect( Rectf( trackRect.getUpperLeft(), rect.getUpperRight() ) );
    
    if ( mIsMouseOnTrack ) ci::gl::color( mBgOverColor ); else ci::gl::color( mBgColor );
    ci::gl::drawSolidRect( mRect );
    
    for( size_t k=0; k < itemsInWindow.size(); k ++ )
    {
        itemRef          = itemsInWindow[k];

        // render module
        itemRect = makeRect( trackRect, timeWindow, itemRef->getStartTime(), itemRef->getEndTime() );
        itemRef->setRect( itemRect );
        
        // module render only use the module rect without calculating any offset or returning another rect, it's a bit special :]
        QTimelineParamRef nullPtr;
        itemRef->render( mMouseOnItem == itemRef && mMouseOnItem->isMouseOnParam( nullPtr ) );
        
        ci::Rectf r = itemRect;
        
        if ( mIsTrackOpen )
        {
            std::vector<QTimelineParamRef> params = itemRef->getParams();
            
            // render params
            for( size_t i=0; i < params.size(); i++ )
                r = params[i]->render( r );
            
            // render keyframes
            for( size_t i=0; i < params.size(); i++ )
                params[i]->renderKeyframes();
        }
    }
    
    return mRectPaddedHeight;
}


bool QTimelineTrack::mouseDown( ci::app::MouseEvent event )
{
    mSelectedItem = mMouseOnItem;
    mMouseDownPos   = event.getPos();
    
    if ( mSelectedItem )
    {
        if (    mSelectedItem->contains( event.getPos() ) && !mSelectedItem->isMouseOnParam() &&
                event.isLeftDown() && getElapsedSeconds() - mMouseDownAt < 0.5f )
        {
            toggle();
            mMouseOnItem.reset();
            mSelectedItem.reset();
            return true;
        }
        
        mMouseDownAt = getElapsedSeconds();
        
        mSelectedItem->mouseDown( event );
        return true;
    }
    
    else if ( contains( mMouseDownPos ) )
    {
        if ( event.isRightDown() )
            QTimeline::getRef()->openMenu( mMenu, event.getPos() );
        
        else if ( event.isLeftDown() && mMenu->isVisible() )
            QTimeline::getRef()->closeMenu( mMenu );
    }
    
    return false;
}


bool QTimelineTrack::mouseUp( ci::app::MouseEvent event )
{
    if ( mSelectedItem )
        mSelectedItem->mouseUp( event );
    
    mSelectedItem.reset();
    
    mouseMove(event);
    
    return false;
}


bool QTimelineTrack::mouseMove( ci::app::MouseEvent event )
{    
    mIsMouseOnTrack     = false;
    
    if ( mMouseOnItem )
    {
        mMouseOnItem->mouseMove( event );
        mMouseOnItem.reset();
    }
    
    if ( contains( event.getPos() ) )
    {
        mIsMouseOnTrack = true;

        for( size_t k=0; k < mModules.size(); k++ )
            if ( mModules[k]->mouseMove( event ) )
            {
                mMouseOnItem = mModules[k];
                break;
            }
    }

    mMousePrevPos = event.getPos();
    
    return mIsMouseOnTrack;
}


bool QTimelineTrack::mouseDrag( ci::app::MouseEvent event )
{
    if ( mSelectedItem )
        mSelectedItem->mouseDrag( event );
    
    return false;
}


void QTimelineTrack::addModuleItem( float startTime, float duration, QTimelineModuleRef targetRef )
{
    // TODO add module should always ensure that no other modules exist with the same name
    // perhaps QTimelineModule and QTimelineModuleItem should share a unique ID to be both referred with.
    // would help to sort out part of the callbacks mess.

    startTime   = QTimeline::getRef()->snapTime( startTime );
    duration    = QTimeline::getRef()->snapTime( duration );
    
    TimelineRef timelineRef = QTimeline::getRef()->getTimelineRef();
    
    QTimelineModuleItemRef itemRef = QTimelineModuleItem::create( startTime, duration, targetRef, getRef(), timelineRef.get() );
    targetRef->setItemRef( itemRef );
    timelineRef->insert( itemRef );
    
    mModules.push_back( itemRef );
    
    sort( mModules.begin(), mModules.end(), sortModulesHelper );
}


void QTimelineTrack::addAudioItem( float startTime, float duration, string audioTrackFilename )
{
    startTime   = QTimeline::getRef()->snapTime( startTime );
    duration    = QTimeline::getRef()->snapTime( duration );
    
    TimelineRef timelineRef = QTimeline::getRef()->getTimelineRef();
    
    QTimelineAudioItemRef itemRef = QTimelineAudioItem::create( startTime, duration, getRef(), timelineRef.get() );
    timelineRef->insert( itemRef );
    
    mModules.push_back( itemRef );
    
    sort( mModules.begin(), mModules.end(), sortModulesHelper );
}


void QTimelineTrack::markModuleForRemoval( QTimelineItemRef moduleItemRef )
{
    for( size_t k=0; k < mModules.size(); k++ )
        if ( mModules[k] == moduleItemRef )
        {
            QTimeline::getRef()->markModuleForRemoval( moduleItemRef );
            return;
        }
    
    /*
    for( size_t k=0; k < mModules.size(); k++ )
        if ( mModules[k] == moduleItemRef )
        {
            mQTimeline->mTimeline->remove( mModules[k] );                                       // remove() flag the item as erase marked, timeline::stepTo() is in charge to actually delete the item
            mQTimeline->updateCurrentTime();                                                    // updateCurrentTime() force the call to stepTo()
            mQTimeline->callDeleteModuleCb( mModules[k]->getName(), mModules[k]->getType() );   // callback to delete the QTimelineModule
            mModules.erase( mModules.begin()+k );                                               // remove the QTimelineModuleItemRef
            return;
        }
     */
}


XmlTree QTimelineTrack::getXmlNode()
{
    XmlTree node( "track", "" );
    node.setAttribute( "name", getName() );

    for( size_t k=0; k < mModules.size(); k++ )
        node.push_back( mModules[k]->getXmlNode() );
    
    return node;
}


void QTimelineTrack::loadXmlNode( ci::XmlTree node )
{
    setName( node.getAttributeValue<string>( "name" ) );
    
    string  name, type, targetModuleType, audioTrackFilename;
    float   startTime, duration;
    
    for( XmlTree::Iter nodeIt = node.begin("item"); nodeIt != node.end(); ++nodeIt )
    {
        name                = nodeIt->getAttributeValue<string>( "name" );
        type                = nodeIt->getAttributeValue<string>( "type" );
        startTime           = nodeIt->getAttributeValue<float>( "startTime" );
        duration            = nodeIt->getAttributeValue<float>( "duration" );

        if ( type == "QTimelineModuleItem" )
        {
            targetModuleType    = nodeIt->getAttributeValue<string>( "targetModuleType" );
            QTimeline::getRef()->callCreateModuleCb( name, targetModuleType, startTime, duration, getRef() );
        }
        
        else if ( type == "QTimelineAudioItem" )
        {
            audioTrackFilename = nodeIt->getAttributeValue<string>( "trackPath" );
            addAudioItem( startTime, duration, audioTrackFilename );
        }
        
        for( size_t k=0; k < mModules.size(); k++ )
            if ( mModules[k]->getName() == name && mModules[k]->getType() == type )
                mModules[k]->loadXmlNode( *nodeIt );
    }

    sort( mModules.begin(), mModules.end(), sortModulesHelper );
}


void QTimelineTrack::menuEventHandler( QTimelineMenuItemRef item )
{
    QTimeline   *timelineRef = QTimeline::getRef();
    
    if ( item->getMeta() == "create_module_item" )
        timelineRef->callCreateModuleCb( "untitled", item->getName(), timelineRef->getTimeFromPos( mMouseDownPos.x ), 2.0f, getRef() );
    
    else if ( item->getMeta() == "create_audio_item" )
    {
        addAudioItem( timelineRef->getTimeFromPos( mMouseDownPos.x ), 2.0f );
        console() << "create audio track" << endl;
    }
  
    else if ( ( item->getMeta() == "new_track_above" ) || ( item->getMeta() == "new_track_below" ) )
    {
        QTimelineTrackRef ref( new QTimelineTrack( "track untitled" ) );
        
        if ( item->getMeta() == "new_track_above" )
            timelineRef->addTrack( ref, getRef(), true );
        else
            timelineRef->addTrack( ref, getRef(), false );
    }
    
    timelineRef->closeMenu( mMenu );
}


void QTimelineTrack::initMenu()
{
    QTimeline   *timelineRef = QTimeline::getRef();
    
    mMenu->init( "TRACK MENU" );
    
    mMenu->addButton("New track above", "new_track_above", this, &QTimelineTrack::menuEventHandler);
    mMenu->addButton("New track below", "new_track_below", this, &QTimelineTrack::menuEventHandler);
    
    mMenu->addSeparator();
    mMenu->addLabel( "Module items" );
    mMenu->addSeparator();
    
    map<string, QTimeline::ModuleCallbacks>::iterator it;
    for ( it=timelineRef->mModuleCallbacks.begin() ; it != timelineRef->mModuleCallbacks.end(); it++ )
        mMenu->addButton( it->first, "create_module_item", this, &QTimelineTrack::menuEventHandler );
    
    mMenu->addSeparator();
    mMenu->addLabel( "Audio items" );
    mMenu->addButton( "new audio item", "create_audio_item", this, &QTimelineTrack::menuEventHandler );
    mMenu->addSeparator();
    

}


void QTimelineTrack::eraseModule( QTimelineItemRef itemRef )
{
    mSelectedItem.reset();
    mMouseOnItem.reset();
    
    for( size_t k=0; k < mModules.size(); k++ )
        if ( mModules[k] == itemRef )
        {
            mModules.erase( mModules.begin()+k );
            return;
        }
}


void QTimelineTrack::findModuleBoundaries( QTimelineItemRef itemRef, float *prevEndTime, float *nextStartTime )
{
    QTimelineItemRef            prevModule, nextModule;
    
    for( size_t k=0; k < mModules.size(); k++ )
    {
        if ( mModules[k].get() != itemRef.get() )
            continue;
        
        if ( k > 0 )
            prevModule = mModules[k-1];
        
        if ( k != mModules.size() - 1 )
            nextModule = mModules[k+1];
        
        break;
    }
    
    *prevEndTime    = prevModule ? prevModule->getEndTime()     : 0.0f;
    *nextStartTime  = nextModule ? nextModule->getStartTime()   : 10000;    // max bound should be the timeline or cue end time
}


