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
    QTimeline *timelineRef = QTimeline::getPtr();
    
    if ( mMenu )
        timelineRef->closeMenu( mMenu );
    
    for( size_t k=0; k < mItems.size(); k++ )
        timelineRef->markItemForRemoval( mItems[k] );
    
    timelineRef->eraseMarkedItems();

    timelineRef = NULL;
}


ci::Rectf QTimelineTrack::render( ci::Rectf rect, ci::Vec2f timeWindow, double currentTime )
{
    std::vector<QTimelineItemRef>   itemsInWindow;
    QTimelineItemRef                itemRef;
    
    ci::Rectf                       trackRect, itemRect;
    
    float maxHeight = 0.0f;
    float h;
    
    // get all modules in time window and calculate the max height
    for( size_t k=0; k < mItems.size(); k ++ )
        if ( mItems[k]->isInWindow( timeWindow ) )
        {
            itemsInWindow.push_back( mItems[k] );
            h = mItems[k]->getHeight();
            if ( h > maxHeight )
                maxHeight = h;
        }
    
    trackRect = Rectf( rect.getUpperLeft() - Vec2f( 0, TIMELINE_ITEM_HEIGHT ), rect.getUpperRight() );
    
    if ( mIsTrackOpen )    
        trackRect.offset( Vec2f( 0, - maxHeight + TIMELINE_ITEM_HEIGHT ) );
    
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
            QTimeline::getPtr()->openMenu( mMenu, event.getPos() );
        
        else if ( event.isLeftDown() && mMenu->isVisible() )
            QTimeline::getPtr()->closeMenu( mMenu );
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

        for( size_t k=0; k < mItems.size(); k++ )
            if ( mItems[k]->mouseMove( event ) )
            {
                mMouseOnItem = mItems[k];
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


QTimelineItemRef QTimelineTrack::addModuleItem( float startTime, float duration, std::string name )
{
    startTime   = QTimeline::getPtr()->snapTime( startTime );
    duration    = QTimeline::getPtr()->snapTime( duration );
    
    TimelineRef             timelineRef = QTimeline::getPtr()->getTimelineRef();
    
    QTimelineModuleItemRef  itemRef     = QTimelineModuleItem::create( name, startTime, duration, getRef() );
    
    timelineRef->insert( itemRef );
    mItems.push_back( itemRef );
    
    sort( mItems.begin(), mItems.end(), sortModulesHelper );
    
    return itemRef;
}


QTimelineItemRef QTimelineTrack::addAudioItem( float startTime, float duration, string filename )
{
    startTime   = QTimeline::getPtr()->snapTime( startTime );
    duration    = QTimeline::getPtr()->snapTime( duration );
    
    TimelineRef timelineRef = QTimeline::getPtr()->getTimelineRef();
    
    QTimelineAudioItemRef itemRef = QTimelineAudioItem::create( startTime, duration, filename, getRef(), timelineRef.get() );
    timelineRef->insert( itemRef );
    
    mItems.push_back( itemRef );
    
    sort( mItems.begin(), mItems.end(), sortModulesHelper );
    
    return itemRef;
}


XmlTree QTimelineTrack::getXmlNode()
{
    XmlTree node( "track", "" );
    node.setAttribute( "name", getName() );

    for( size_t k=0; k < mItems.size(); k++ )
        node.push_back( mItems[k]->getXmlNode() );
    
    return node;
}


void QTimelineTrack::loadXmlNode( ci::XmlTree node )
{
    setName( node.getAttributeValue<string>( "name" ) );
    
    string              name, type, targetModuleType, trackFilename;
    float               startTime, duration;
    
    for( XmlTree::Iter nodeIt = node.begin("item"); nodeIt != node.end(); ++nodeIt )
    {
        name                = nodeIt->getAttributeValue<string>( "name" );
        type                = nodeIt->getAttributeValue<string>( "type" );
        startTime           = nodeIt->getAttributeValue<float>( "startTime" );
        duration            = nodeIt->getAttributeValue<float>( "duration" );
                
        if ( type == "QTimelineModuleItem" )
        {
            targetModuleType                = nodeIt->getAttributeValue<string>( "targetModuleType" );
            QTimelineItemRef    itemRef     = addModuleItem( startTime, duration, name );
            QTimeline::getPtr()->callCreateModuleCb( targetModuleType, itemRef );
            
            itemRef->loadXmlNode( *nodeIt );
        }
        
        else if ( type == "QTimelineAudioItem" )
        {
            trackFilename = nodeIt->getAttributeValue<string>( "filename" );
            QTimelineItemRef    itemRef     = addAudioItem( startTime, duration, trackFilename );
            
            itemRef->loadXmlNode( *nodeIt );
        }
    }
    
    sort( mItems.begin(), mItems.end(), sortModulesHelper );
}


void QTimelineTrack::menuEventHandler( QTimelineMenuItemRef item )
{
    QTimeline   *timelineRef = QTimeline::getPtr();
    
    if ( item->getMeta() == "create_module_item" )
    {
        float               startTime   = timelineRef->snapTime( timelineRef->getTimeFromPos( mMouseDownPos.x ) );
        float               duration    = 2.0f;
        string              name        = "untitled";
        QTimelineItemRef    itemRef     = addModuleItem( startTime, duration, name );
        QTimeline::getPtr()->callCreateModuleCb( item->getName(), itemRef );
    }
    
    else if ( item->getMeta() == "create_audio_item" )
        addAudioItem( timelineRef->getTimeFromPos( mMouseDownPos.x ), 2.0f, item->getName() );
  
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
    QTimeline   *timelineRef = QTimeline::getPtr();
    
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
    
    fs::path filePath;
    fs::path audioDir = getAssetPath("audio");
    fs::directory_iterator endIt;
    
    for( fs::directory_iterator dirIt( audioDir ) ; dirIt != endIt ; ++dirIt )
    {
        filePath = dirIt->path();
        if ( QTimelineAudioItem::supportedFormats.isSupported( filePath.extension().generic_string() ) )
            mMenu->addButton( filePath.filename().generic_string(), "create_audio_item", this, &QTimelineTrack::menuEventHandler );
    }

    mMenu->addSeparator();
}


void QTimelineTrack::eraseModule( QTimelineItemRef itemRef )
{
    mSelectedItem.reset();
    mMouseOnItem.reset();
    
    for( size_t k=0; k < mItems.size(); k++ )
        if ( mItems[k] == itemRef )
        {
            mItems.erase( mItems.begin()+k );
            return;
        }
}


void QTimelineTrack::findModuleBoundaries( QTimelineItemRef itemRef, float *prevEndTime, float *nextStartTime )
{
    QTimelineItemRef            prevModule, nextModule;
    
    for( size_t k=0; k < mItems.size(); k++ )
    {
        if ( mItems[k].get() != itemRef.get() )
            continue;
        
        if ( k > 0 )
            prevModule = mItems[k-1];
        
        if ( k != mItems.size() - 1 )
            nextModule = mItems[k+1];
        
        break;
    }
    
    *prevEndTime    = prevModule ? prevModule->getEndTime()     : 0.0f;
    *nextStartTime  = nextModule ? nextModule->getStartTime()   : 10000;    // max bound should be the timeline or cue end time
}

