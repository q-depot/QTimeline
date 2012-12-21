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


QTimelineModuleItem::QTimelineModuleItem( QTimelineModule *targetModule, float startAt, float duration, QTimelineTrackRef trackRef, Timeline *parent )
: QTimelineWidgetWithHandles( targetModule->getName() )
{
    setAutoRemove(false);
    
    mTargetModuleRef    = QTimelineModuleRef( targetModule );
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
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k].reset();
    
    if ( mMenu )
        mParentTrack->mQTimeline->closeMenu( mMenu );
    
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
    
    mParams.push_back( QTimelineParamRef( new QTimelineParam( this, name, var, minVal, maxVal, getStartTime() ) ) );
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
//    mFont->drawString( getName(), mRect.getCenter() + mNameStrSize * Vec2f( -0.5f, 0.3f ) );
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
    mMousePrevPos   = event.getPos();
    
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
        if (mParentTrack->contains(Vec2f(event.getX(), event.getY())))
        {
            float deltaT       = mParentTrack->mQTimeline->getPxInSeconds( event.getPos().x - mMousePrevPos.x );
            float              prevModuleEndTime, nextModuleStartTime;
            
            findModuleBoundaries( &prevModuleEndTime, &nextModuleStartTime );
            
            // drag handles
            if ( handlesMouseDrag( deltaT, prevModuleEndTime, nextModuleStartTime ) )
                mParent->reset();
            
            // drag module
            else
                dragWidget( deltaT, prevModuleEndTime, nextModuleStartTime );
                    
            mParentTrack->mQTimeline->updateCurrentTime();
        } else {
            changeTrack(event.getY() < mParentTrack->getRect().y1);
        }
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


void QTimelineModuleItem::dragHandle( float deltaT, float prevEndTime, float nextStartTime  )
{
    if ( mSelectedHandleType == LEFT_HANDLE )
    {
        float duration = math<float>::clamp( getDuration() - deltaT, mParentTrack->mQTimeline->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), getEndTime() - prevEndTime );
        setStartTime( getEndTime() - duration );
        setDuration( duration );
    }

    else if ( mSelectedHandleType == RIGHT_HANDLE )
    {
        float duration = math<float>::clamp( getDuration() + deltaT, mParentTrack->mQTimeline->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), nextStartTime - getStartTime() );
        setDuration( duration );
    }
  
    updateLabel();
    
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateLabel();
}


void QTimelineModuleItem::dragWidget( float deltaT, float prevEndTime, float nextStartTime )
{
    float startTime = getStartTime();
    float time      = math<float>::clamp( startTime + deltaT, prevEndTime, nextStartTime - getDuration() );
    
    setStartTime( time );
    
    // update params, keyframes move with the module
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateKeyframesPos( time - startTime );
}


XmlTree QTimelineModuleItem::getXmlNode()
{
    XmlTree node( "module", "" );
    node.setAttribute( "name", mName );
    
    for( size_t k=0; k < mParams.size(); k++ )
        node.push_back( mParams[k]->getXmlNode() );
    
    return node;
}


void QTimelineModuleItem::loadXmlNode( ci::XmlTree node )
{
}

void QTimelineModuleItem::changeTrack(bool moveup)
{
    for (auto i = mParentTrack->mQTimeline->getTracks().begin();
         i != mParentTrack->mQTimeline->getTracks().end(); i++)
    {
        if (i->get() == mParentTrack.get())
        {
            if (moveup)
            {
                if (i == mParentTrack->mQTimeline->getTracks().begin())
                    return;
                i--;
            } else {
                if (i == mParentTrack->mQTimeline->getTracks().end()-1)
                    return;
                i++;
            }
            QTimelineModuleItemRef me = thisRef();
            mParentTrack->deleteModuleItem(thisRef(), false);
            mParentTrack = *i;
            mParentTrack->addModuleItem(thisRef());
            
            float prevModuleEndTime, nextModuleStartTime;
            findModuleBoundaries( &prevModuleEndTime, &nextModuleStartTime );
            float deltaT = 0;
            dragWidget( deltaT, prevModuleEndTime, nextModuleStartTime );
            
            mParentTrack->mQTimeline->update();
            return;
        }
    }
}

void QTimelineModuleItem::menuEventHandler( QTimelineMenuItem* item )
{
    if ( item->getMeta() == "delete" )
    {
        mParentTrack->deleteModuleItem( thisRef() );
//      mParentTrack->mQTimeline->callDeleteModuleCb( mTargetModuleRef->getType(), getName(), mMouseDownPos );
        mParentTrack->mQTimeline->closeMenu( mMenu );
    }
    if ((item->getMeta() == "moveup" ) || (item->getMeta() == "movedown"))
    {
        mParentTrack->mQTimeline->closeMenu( mMenu );
        changeTrack(item->getMeta() == "moveup");
    }
}


void QTimelineModuleItem::initMenu()
{
    mMenu->init( "MODULE MENU" );
    
    mMenu->addItem( "X DELETE", "delete", this, &QTimelineModuleItem::menuEventHandler );
    mMenu->addItem( "Move up a track", "moveup", this, &QTimelineModuleItem::menuEventHandler);
    mMenu->addItem( "Move down a track", "movedown", this, &QTimelineModuleItem::menuEventHandler);
}

