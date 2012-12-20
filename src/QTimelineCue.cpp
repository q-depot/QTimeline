/*
 *  QTimelineCue.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimeline.h"
#include "QTimelineCueManager.h"
#include "QTimelineCue.h"


using namespace ci;
using namespace ci::app;
using namespace std;


QTimelineCue::QTimelineCue( QTimeline *qTimeline, QTimelineCueManager *cueManager, string name, double startTime, double duration )
: mQTimeline(qTimeline), mCueManager(cueManager), mName(name), mStartTime(startTime), mDuration(duration), QTimelineWidgetWithHandles(name)
{
    mBgColor            = QTimeline::mCueBgCol;
    mBgOverColor        = QTimeline::mCueBgOverCol;
    mTextColor          = QTimeline::mCueTextCol;
    mHandleColor        = QTimeline::mCueHandleCol;
    mHandleOverColor    = QTimeline::mCueHandleOverCol;

    mFont               = ci::gl::TextureFont::create( ci::Font( "Helvetica", 14 ) );

    mIsMouseOn          = false;
    
    // init rect width
    setRect( Rectf( mQTimeline->getPosFromTime( getStartTime() ), 0, mQTimeline->getPosFromTime( getEndTime() ), 0 ) );
    
    updateLabel();
    
    initMenu();
}

void QTimelineCue::render()
{
//    if ( mIsMouseOn ) gl::color( mBgOverColor ); else gl::color( mBgColor );
//    gl::drawSolidRect( mRect );
    glBegin( GL_QUADS );
    gl::color( mBgColor );          gl::vertex( mRect.getUpperLeft() );
    gl::color( mBgColor );          gl::vertex( mRect.getUpperRight() );
    gl::color( mBgColor * 0.8f );   gl::vertex( mRect.getLowerRight() );
    gl::color( mBgColor * 0.8f );   gl::vertex( mRect.getLowerLeft() );
    glEnd();
    
    if ( mQTimeline->mPlayMode == QTimeline::CUE_LIST )
    {
        float timeNorm = ( mQTimeline->getTime() - getStartTime() ) / ( getEndTime() - getStartTime() );
        if ( timeNorm >= 0.0f && timeNorm <= 1.0f )
        {
            gl::color( mFgColor );
            gl::drawSolidRect( Rectf( mRect.getUpperLeft(), Vec2f( mRect.x1 + mRect.getWidth() * timeNorm, mRect.y2 ) ) );
        }
    }
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.08f ) );
    glBegin( GL_LINE_STRIP );
    gl::vertex( mRect.getLowerLeft() );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    glEnd();
    
    gl::color( mTextColor );
    mFont->drawString( mLabel, mRect.getUpperLeft() + Vec2f( TIMELINE_MODULE_HANDLE_WIDTH * 2, 19.0f ) );

    if ( mIsMouseOn )
        renderHandles();
}

bool QTimelineCue::mouseMove( MouseEvent event )
{
    mMouseOnHandleType  = NO_HANDLE;
    mIsMouseOn          = false;
    
    if ( mRect.contains( event.getPos() ) )
    {
        handlesMouseMove( event.getPos() );
        mIsMouseOn = true;
    }
    
    return mIsMouseOn;
}


bool QTimelineCue::mouseDown( MouseEvent event )
{
    if ( !mIsMouseOn )
        return false;
    
    if ( event.isRightDown() )
        mQTimeline->openMenu( mMenu, event.getPos() );
    
    mMousePrevPos   = event.getPos();
    
    handlesMouseDown();
    
    return true;
}


bool QTimelineCue::mouseUp( MouseEvent event )
{
    mouseMove( event );
    
    return false;
}


bool QTimelineCue::mouseDrag( MouseEvent event )
{
    if ( !mIsMouseOn )
        return false;
    
    float deltaT       = mQTimeline->getPxInSeconds( event.getPos().x - mMousePrevPos.x );
    float              prevCueEndTime, nextCueStartTime;

    findModuleBoundaries( &prevCueEndTime, &nextCueStartTime );

    if ( !handlesMouseDrag( deltaT, prevCueEndTime, nextCueStartTime ) )      // drag handles
      dragWidget( deltaT, prevCueEndTime, nextCueStartTime );                 // drag module

    mMousePrevPos = event.getPos();

    return true;
}


void QTimelineCue::dragWidget( float deltaT, float prevEndTime, float nextStartTime  )
{
    mStartTime = math<float>::clamp( mStartTime + deltaT, prevEndTime, nextStartTime - mDuration );
}


void QTimelineCue::dragHandle( float deltaT, float prevEndTime, float nextStartTime  )
{
    if ( mSelectedHandleType == LEFT_HANDLE )
    {
        float duration  = math<float>::clamp( mDuration - deltaT, mQTimeline->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), getEndTime() - prevEndTime );
        mStartTime      = getEndTime() - duration;
        mDuration       = duration;
    }

    else if ( mSelectedHandleType == RIGHT_HANDLE )
    {
        float duration  = math<float>::clamp( mDuration + deltaT, mQTimeline->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), nextStartTime - getStartTime() );
        mDuration       = duration;
    }
    
    updateLabel();
}


void QTimelineCue::findModuleBoundaries( float *prevEndTime, float *nextStartTime )
{

    QTimelineCueRef prevCue;
    QTimelineCueRef nextCue;
    
    for( size_t k=0; k < mCueManager->mCueList.size(); k++ )
    {
        if ( mCueManager->mCueList[k].get() != this )
            continue;

        if ( k > 0 )
            prevCue = mCueManager->mCueList[k-1];
        
        if ( k != mCueManager->mCueList.size() - 1 )
            nextCue = mCueManager->mCueList[k+1];
        
        break;
    }

    *prevEndTime    = prevCue ? prevCue->getEndTime()     : 0.0f;
    *nextStartTime  = nextCue ? nextCue->getStartTime()   : 10000;    // max bound should be the timeline or cue end time

}


void QTimelineCue::menuEventHandler( QTimelineMenuItem* item )
{
    mQTimeline->closeMenu( mMenu );
    mCueManager->deleteCue( this );
}


void QTimelineCue::initMenu()
{
    mMenu->init( "CUE MENU" );
    
    mMenu->addItem( "DELETE", "", this, &QTimelineCue::menuEventHandler );
}


void QTimelineCue::updateLabel()
{
    mLabel = mName;

    if ( mLabel == "" )
        return;
    
    int charsN = mLabel.size() * ( max( 0.0f, mRect.getWidth() - TIMELINE_MODULE_HANDLE_WIDTH * 4 ) ) / mNameStrSize.x;

    if ( charsN < mLabel.size() )
        mLabel.resize( charsN );

    mLabelStrSize = mFont->measureString( mLabel );
}
