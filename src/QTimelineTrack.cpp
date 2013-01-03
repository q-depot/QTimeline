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

using namespace ci;
using namespace ci::app;
using namespace std;

bool sortModulesHelper( QTimelineItemRef a, QTimelineItemRef b ) { return ( a->getStartTime() < b->getStartTime() ); }


QTimelineTrack::QTimelineTrack(  QTimeline *timeline, string name ) : mQTimeline(timeline), QTimelineWidget( name )
{
    mBgColor            = QTimeline::mTracksBgCol;
    mBgOverColor        = QTimeline::mTracksBgCol;
    
    mIsMouseOnTrack     = false;
    mIsTrackOpen        = true;
    
    mMouseDownAt        = 0.0f;
    
    initMenu();
}


QTimelineTrack::~QTimelineTrack()
{
    if ( mMenu )
        mQTimeline->closeMenu( mMenu );
    
//    for( size_t k=0; k < mModules.size(); k++ )
//    {
//        mQTimeline->mTimeline->remove( mModules[k] );                                       // remove() flag the item as erase marked, timeline::stepTo() is in charge to actually delete the item
//        mQTimeline->updateCurrentTime();                                                    // updateCurrentTime() force the call to stepTo()
//        mQTimeline->callDeleteModuleCb( mModules[k]->getName(), mModules[k]->getType() );   // callback to delete the QTimelineModule
//    }
//    mModules.clear();
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
        
        itemRef->setRect( Rectf( itemRect.getUpperLeft(), r.getLowerRight() ) );
    }
    
    return mRectPaddedHeight;
}


bool QTimelineTrack::mouseDown( ci::app::MouseEvent event )
{
    mSelectedItem = mMouseOnItem;
    mMouseDownPos   = event.getPos();
    
    if ( mSelectedItem )
    {
        if ( mSelectedItem->contains( event.getPos() ) && !mSelectedItem->isMouseOnParam() && event.isLeftDown() && getElapsedSeconds() - mMouseDownAt < 0.5f )
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
    if ( mSelectedItem )
        mSelectedItem->mouseUp( event );
    
    mSelectedItem.reset();
    
//    mMouseOnItem.reset();
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


Vec2f QTimelineTrack::getTimeWindow()
{
    return mQTimeline->getTimeWindow();
}


void QTimelineTrack::addModuleItem( QTimelineModuleRef targetRef, float startTime, float duration )
{
    // TODO add module should always ensure that no other modules exist with the same name
    // perhaps QTimelineModule and QTimelineModuleItem should share a unique ID to be both referred with.
    // would help to sort out part of the callbacks mess.

    startTime   = mQTimeline->snapTime( startTime );
    duration    = mQTimeline->snapTime( duration );
    
    TimelineRef timelineRef = mQTimeline->getTimelineRef();
    
    QTimelineModuleItemRef itemRef = QTimelineModuleItem::create( startTime, duration, targetRef, getRef(), timelineRef.get() );
    targetRef->setItemRef( itemRef );
    timelineRef->insert( itemRef );
    
    mModules.push_back( itemRef );
    
    sort( mModules.begin(), mModules.end(), sortModulesHelper );
}


void QTimelineTrack::addAudioItem( float startTime, float duration )
{
    startTime   = mQTimeline->snapTime( startTime );
    duration    = mQTimeline->snapTime( duration );
    
    TimelineRef timelineRef = mQTimeline->getTimelineRef();
    
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
            mQTimeline->markModuleForRemoval( moduleItemRef );
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
    node.setAttribute( "name", mName );

    for( size_t k=0; k < mModules.size(); k++ )
        node.push_back( mModules[k]->getXmlNode() );
    
    return node;
}


void QTimelineTrack::loadXmlNode( ci::XmlTree node )
{
    setName( node.getAttributeValue<string>( "name" ) );
    
    string  name, type;
    float   startTime, duration;
    
    for( XmlTree::Iter nodeIt = node.begin("module"); nodeIt != node.end(); ++nodeIt )
    {
        name       = nodeIt->getAttributeValue<string>( "name" );
        type       = nodeIt->getAttributeValue<string>( "type" );
        startTime  = nodeIt->getAttributeValue<float>( "startTime" );
        duration   = nodeIt->getAttributeValue<float>( "duration" );

        mQTimeline->callCreateModuleCb( name, type, startTime, duration, getRef() );
        
    for( size_t k=0; k < mModules.size(); k++ )
        if ( mModules[k]->getName() == name && mModules[k]->getType() == type )
            mModules[k]->loadXmlNode( *nodeIt );
    }

    sort( mModules.begin(), mModules.end(), sortModulesHelper );
}


void QTimelineTrack::menuEventHandler( QTimelineMenuItemRef item )
{
    if ( item->getMeta() == "create_module_item" )
    {
        mQTimeline->callCreateModuleCb( "untitled", item->getName(), mQTimeline->getTimeFromPos( mMouseDownPos.x ), 2.0f, getRef() );
        mQTimeline->closeMenu( mMenu );
    }
    
    else if ( item->getMeta() == "create_audio_item" )
    {
        addAudioItem( QTimeline::getRef()->getTimeFromPos( mMouseDownPos.x ), 2.0f );
        mQTimeline->closeMenu( mMenu );
        
        console() << "create audio track" << endl;
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
    
    mMenu->addButton("New track above", "new_track_above", this, &QTimelineTrack::menuEventHandler);
    mMenu->addButton("New track below", "new_track_below", this, &QTimelineTrack::menuEventHandler);
    
    mMenu->addSeparator();
    mMenu->addLabel( "Module items" );
    mMenu->addSeparator();
    
    map<string, QTimeline::ModuleCallbacks>::iterator it;
    for ( it=mQTimeline->mModuleCallbacks.begin() ; it != mQTimeline->mModuleCallbacks.end(); it++ )
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
            // force to release params that refer to the module itself, otherwise the deconstructor never gets called
            // this might be an issue with the actual class design, perhaps I should remove the module ref from the params
            mModules[k]->clear();
            mModules.erase( mModules.begin()+k );
            return;
        }
}

