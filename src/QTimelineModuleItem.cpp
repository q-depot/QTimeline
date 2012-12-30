/*
 *  QTimelineModuleItem.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/Timeline.h"

#include "QTimeline.h"
#include "QTimelineModuleItem.h"
#include "QTimelineParam.h"
#include "QTimelineTrack.h"
#include "QTimelineModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


QTimelineModuleItem::QTimelineModuleItem( QTimelineModuleRef targetRef, float startAt, float duration, QTimelineTrackRef trackRef, Timeline *parent )
: QTimelineWidgetWithHandles( targetRef->getName() )
{
    setAutoRemove(false);
    
    mTargetModuleRef    = targetRef;
    mParentTrack        = trackRef;
    mParent             = parent;
    
    setStartTime( startAt );
    setDuration( duration );
    
    mBgColor            = QTimeline::mModulesBgCol;
	mBgOverColor        = QTimeline::mModulesBgOverCol;
    mTextColor          = QTimeline::mModulesTextCol;
    mHandleColor        = QTimeline::mModulesHandleCol;
    mHandleOverColor    = QTimeline::mModulesHandleOverCol;

    // init rect width
    setRect( Rectf( mParentTrack->mQTimeline->getPosFromTime( getStartTime() ), 0,
                    mParentTrack->mQTimeline->getPosFromTime( getEndTime() ), 0 ) );
    
    updateLabel();
    
    initMenu();
}


QTimelineModuleItem::~QTimelineModuleItem()
{
    console() << "delete QTimelineModuleItem" << endl;
    
    clear();
}


void QTimelineModuleItem::clear()
{
    if ( mMenu )
        mParentTrack->mQTimeline->closeMenu( mMenu );
    
    mParams.clear();
}


void QTimelineModuleItem::update( float relativeTime )
{
    if ( isComplete() )
        return;
    
    // update values
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->update( relativeTime );
    
    mTargetModuleRef->update();
}


void QTimelineModuleItem::registerParam( const string name, float initVal, float minVal, float maxVal )
{
    registerParam( name, new float(initVal), minVal, maxVal );
}


void QTimelineModuleItem::registerParam( const std::string name, float *var, float minVal, float maxVal )
{
    for( size_t k=0; k < mParams.size(); k++ )
        if ( mParams[k]->getName() == name )
            mParams[k]->mVar = var;
    
    mParams.push_back( QTimelineParamRef( new QTimelineParam( thisRef(), name, var, minVal, maxVal, getStartTime() ) ) );
}


float QTimelineModuleItem::getParamValue( const std::string &name )
{
    for( size_t k=0; k < mParams.size(); k++ )
        if ( mParams[k]->getName() == name )
            return mParams[k]->getValue();
    
    return 0.0f;
}


void QTimelineModuleItem::render( bool mouseOver )
{
    // render bg rect
//    if ( mouseOver ) gl::color( mBgOverColor ); else gl::color( mBgColor );
    
    glBegin( GL_QUADS );
    gl::color( mBgColor );          gl::vertex( mRect.getUpperLeft() );
    gl::color( mBgColor );          gl::vertex( mRect.getUpperRight() );
    gl::color( mBgColor * 0.8f );   gl::vertex( mRect.getLowerRight() );
    gl::color( mBgColor * 0.8f );   gl::vertex( mRect.getLowerLeft() );
    glEnd();
    
    // render handles
    if ( mouseOver )
        renderHandles();
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.08f ) );
    glBegin( GL_LINE_STRIP );
    gl::vertex( mRect.getLowerLeft() );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    glEnd();
    
    // render name
    gl::color( mTextColor );
    mFont->drawString( getLabel(), mRect.getCenter() + mLabelStrSize * Vec2f( -0.5f, 0.3f ) );
}


bool QTimelineModuleItem::mouseMove( MouseEvent event )
{
    if ( mMouseOnParam )
        mMouseOnParam->mouseMove(event);
    
    mMouseOnParam.reset();
    
    mMouseOnHandleType  = NO_HANDLE;
    
    if ( mWidgetRect.contains( event.getPos() ) )
    {
        if ( handlesMouseMove( event.getPos() ) )
            return true;
            
        else if ( isTrackOpen() )
        {
            for( size_t k=0; k < mParams.size(); k++ )
                if ( mParams[k]->mouseMove(event) )
                {
                    mMouseOnParam = mParams[k];
                    return true;
                }
        }
        return true;
    }
    
    return false;
}


bool QTimelineModuleItem::mouseDown( MouseEvent event )
{
    mMouseDownPos       = event.getPos();
    mMouseDownStartTime = getStartTime();
    mMouseDownEndTime   = getEndTime();
    
    if ( mMouseOnParam )
        mMouseOnParam->mouseDown( event );

    else if ( event.isRightDown() )
        mParentTrack->mQTimeline->openMenu( mMenu, event.getPos() );
    
    else
        handlesMouseDown();
    
    return false;
}


bool QTimelineModuleItem::mouseUp( MouseEvent event )
{
    if ( mMouseOnParam )
        mMouseOnParam->mouseUp( event );
  
    mouseMove( event );
    
    return false;
}


bool QTimelineModuleItem::mouseDrag( MouseEvent event )
{
    // drag param to update a keyframe
    if ( mMouseOnParam )
        mMouseOnParam->mouseDrag( event );
    
    else
    {
        if ( !dragHandles( event ) )
            dragWidget( event );
        
        mParentTrack->mQTimeline->updateCurrentTime();
    }
    
    mMousePrevPos = event.getPos();

    return false;
}


bool QTimelineModuleItem::isMouseOnParam( QTimelineParamRef ref ) { return ref == mMouseOnParam; };


bool QTimelineModuleItem::isMouseOnKeyframe( QTimelineParamRef paramRef, QTimelineKeyframeRef keyframeRef ) { return paramRef == mMouseOnParam && paramRef->isMouseOnKeyframe(keyframeRef); };


bool QTimelineModuleItem::isTrackOpen() { return mParentTrack->isOpen(); }


void QTimelineModuleItem::findModuleBoundaries( float *prevEndTime, float *nextStartTime )
{
    QTimelineModuleItemRef  prevModule, nextModule;
    
    for( size_t k=0; k < mParentTrack->mModules.size(); k++ )
    {
        if ( mParentTrack->mModules[k] != thisRef() )
            continue;
        
        if ( k > 0 )
            prevModule = mParentTrack->mModules[k-1];
        
        if ( k != mParentTrack->mModules.size() - 1 )
            nextModule = mParentTrack->mModules[k+1];
        
        break;
    }
    
    *prevEndTime    = prevModule ? prevModule->getEndTime()     : 0.0f;
    *nextStartTime  = nextModule ? nextModule->getStartTime()   : 10000;    // max bound should be the timeline or cue end time
}


bool QTimelineModuleItem::dragHandles( MouseEvent event )
{
    float diff          = mParentTrack->mQTimeline->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float startTime, endTime, prevEndTime, nextStartTime;
    
    findModuleBoundaries( &prevEndTime, &nextStartTime );
    
    if ( mSelectedHandleType == LEFT_HANDLE )
    {
        endTime     = getEndTime();
        startTime   = math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, endTime - mParentTrack->mQTimeline->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ) );
        startTime   = mParentTrack->mQTimeline->snapTime( startTime );
        setStartTime( startTime );
        setDuration( endTime - startTime );
    }

    else if ( mSelectedHandleType == RIGHT_HANDLE )
    {
        startTime   = getStartTime();
        endTime     = math<float>::clamp( mMouseDownEndTime + diff, startTime + mParentTrack->mQTimeline->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), nextStartTime );
        endTime     = mParentTrack->mQTimeline->snapTime( endTime );
        setDuration( endTime - startTime );
    }
  
    updateLabel();
    
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateLabel();
    
    if ( mSelectedHandleType != NO_HANDLE )
        return true;
    
    return false;
}


void QTimelineModuleItem::dragWidget( MouseEvent event )
{
    float diff       = mParentTrack->mQTimeline->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float prevEndTime, nextStartTime;
    
    findModuleBoundaries( &prevEndTime, &nextStartTime );
    
    float startTime = getStartTime();
    float time      = math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, nextStartTime - getDuration() );
    time            = mParentTrack->mQTimeline->snapTime( time );
    
    setStartTime( time );
    
    // update params, keyframes move with the module
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateKeyframesPos( time - startTime );
}


XmlTree QTimelineModuleItem::getXmlNode()
{
    XmlTree node( "module", "" );
    node.setAttribute( "name", getName() );
    node.setAttribute( "type", mTargetModuleRef->getType() );
    node.setAttribute( "startTime", getStartTime() );
    node.setAttribute( "duration",  getDuration() );
    
    for( size_t k=0; k < mParams.size(); k++ )
        node.push_back( mParams[k]->getXmlNode() );
    
    return node;
}


void QTimelineModuleItem::loadXmlNode( ci::XmlTree node )
{
    float   value, time;
    string  fnStr;
    
    for( XmlTree::Iter paramIt = node.begin("param"); paramIt != node.end(); ++paramIt )
    {
        QTimelineParamRef param = findParamByName( paramIt->getAttributeValue<string>( "name" ) );
        
        if ( !param )
            continue;
        
        for( XmlTree::Iter kfIt = paramIt->begin("kf"); kfIt != paramIt->end(); ++kfIt )
        {
            value   = kfIt->getAttributeValue<float>( "value" );
            time    = kfIt->getAttributeValue<float>( "time" );
            fnStr   = kfIt->getAttributeValue<string>( "fn" );
            
            param->addKeyframe( time, value, QTimeline::getEaseFnFromString(fnStr), fnStr );
        }
    }
}


void QTimelineModuleItem::menuEventHandler( QTimelineMenuItemRef item )
{
    if ( item->getMeta() == "delete" )
    {
        mParentTrack->mQTimeline->closeMenu( mMenu );
        mParentTrack->markModuleForRemoval( thisRef() );
    }
    else if ( item->getMeta() == "color_palette" )
    {
        QTimelineMenuColorPalette *palette = (QTimelineMenuColorPalette*)item.get();
        mBgColor = palette->getColor();
    }
}


void QTimelineModuleItem::initMenu()
{
    mMenu->init( "MODULE MENU" );
    
    mMenu->addColorPalette( this, &QTimelineModuleItem::menuEventHandler );
    
    mMenu->addSeparator();
    
    mMenu->addButton( "X DELETE", "delete", this, &QTimelineModuleItem::menuEventHandler );
}


string QTimelineModuleItem::getType()
{
    return mTargetModuleRef->getType();
}


QTimelineParamRef QTimelineModuleItem::findParamByName( std::string name )
{
    for( size_t k=0; k < mParams.size(); k++ )
        if ( mParams[k]->getName() == name )
            return mParams[k];
    
    QTimelineParamRef nullPtr;
    return nullPtr;
}

