/*
 *  QTimelineCue.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
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


QTimelineCue::QTimelineCue( QTimelineCueManager *cueManager, string name, double startTime, double duration )
: mCueManager(cueManager), mStartTime(startTime), mDuration(duration), QTimelineWidgetWithHandles(name)
{
    mBgColor            = QTimeline::mCueBgCol;
    mBgOverColor        = QTimeline::mCueBgOverCol;
    mFgColor            = QTimeline::mCueFgCol;
    mTextColor          = QTimeline::mCueTextCol;
    mHandleColor        = QTimeline::mCueHandleCol;
    mHandleOverColor    = QTimeline::mCueHandleOverCol;

    mFont               = ci::gl::TextureFont::create( ci::Font( "Helvetica", 14 ) );

    mIsMouseOn          = false;
    
    // init rect width
    setRect( Rectf( QTimeline::getPtr()->getPosFromTime( getStartTime() ), 0, QTimeline::getPtr()->getPosFromTime( getEndTime() ), 0 ) );
    
    setName( name );
    
    updateLabel();
    
    initMenu();
}

void QTimelineCue::render()
{
    // bg
    glBegin( GL_QUADS );
    gl::color( mBgColor );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    gl::vertex( mRect.getLowerLeft() );
    glEnd();
    
    // handles
    renderHandles();
    
    // current time bar
    if ( QTimeline::getPtr()->mPlayMode == QTimeline::CUE_LIST )
    {
        float timeNorm = getTimeNorm();
        if ( timeNorm >= 0.0f && timeNorm <= 1.0f )
        {
            float posX = mRect.x1 + mRect.getWidth() * timeNorm;
            gl::color( mFgColor );
            glBegin( GL_QUADS );
            gl::vertex( Vec2i( mRect.x1, mRect.y1 ) );
            gl::vertex( Vec2i( posX, mRect.y1 )     );
            gl::vertex( Vec2i( posX, mRect.y2)      );
            gl::vertex( Vec2i( mRect.x1, mRect.y2 ) );
            glEnd();
        }
    }
    
    // label
    gl::color( mTextColor );
    mFont->drawString( mLabel, mRect.getUpperLeft() + Vec2f( TIMELINE_MODULE_HANDLE_WIDTH * 2, 16.0f ) );
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
    mMouseDownPos       = event.getPos();
    mMouseDownStartTime = getStartTime();
    mMouseDownEndTime   = getEndTime();
    
    if ( !mIsMouseOn )
        return false;
    
    if ( event.isRightDown() )
        QTimeline::getPtr()->openMenu( mMenu, mMouseDownPos );
    
    handlesMouseDown();
    
    mMousePrevPos   = mMouseDownPos;
    
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

    if ( !dragHandles( event ) )
        dragWidget( event );

    mMousePrevPos = event.getPos();

    return true;
}


void QTimelineCue::dragWidget( MouseEvent event )
{    
    float diff       = QTimeline::getPtr()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float prevEndTime, nextStartTime;
    
    findModuleBoundaries( &prevEndTime, &nextStartTime );
    
    float time      = math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, nextStartTime - mDuration );
    time            = QTimeline::getPtr()->snapTime( time );
    
    setStartTime( time );
}


bool QTimelineCue::dragHandles( MouseEvent event )
{
    float diff          = QTimeline::getPtr()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float              prevEndTime, nextStartTime, startTime, endTime;
    
    findModuleBoundaries( &prevEndTime, &nextStartTime );
    
    if ( mSelectedHandleType == LEFT_HANDLE )
    {
        endTime     = getEndTime();
        startTime   = math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, endTime - QTimeline::getPtr()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ) );
        startTime   = QTimeline::getPtr()->snapTime( startTime );
        setStartTime( startTime );
        setDuration( endTime - startTime );
    }
    
    else if ( mSelectedHandleType == RIGHT_HANDLE )
    {
        startTime   = getStartTime();
        endTime     = math<float>::clamp( mMouseDownEndTime + diff, startTime + QTimeline::getPtr()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), nextStartTime );
        endTime     = QTimeline::getPtr()->snapTime( endTime );
        setDuration( endTime - startTime );
    }

    updateLabel();
    
    if ( mSelectedHandleType != NO_HANDLE )
        return true;
    
    return false;
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


void QTimelineCue::menuEventHandler( QTimelineMenuItemRef item )
{
    if ( item->getMeta() == "delete" )
    {
        QTimeline::getPtr()->closeMenu( mMenu );
        mCueManager->deleteCue( this );
    }
    
    else if ( item->getMeta() == "color_palette" )
    {
        QTimelineMenuColorPalette *palette = (QTimelineMenuColorPalette*)item.get();
        setColor( palette->getColor() );
    }
    
    else if ( item->getMeta() == "name_text_box" )
    {
        setName( item->getName() );
        QTimeline::getPtr()->closeMenu( mMenu );
        updateLabel();
    }
}


void QTimelineCue::initMenu()
{
    mMenu->init( "CUE MENU" );

    mMenu->addTextBox( getName(), "name_text_box", this, &QTimelineCue::menuEventHandler );

    mMenu->addColorPalette( this, &QTimelineCue::menuEventHandler );
    
    mMenu->addSeparator();
    
    mMenu->addButton( "X DELETE", "delete", this, &QTimelineCue::menuEventHandler );
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


XmlTree QTimelineCue::getXmlNode()
{
    ColorA col = getColor();
    
    ci::XmlTree node( "cue", "" );
    node.setAttribute( "name",      getName() );
    node.setAttribute( "startTime", getStartTime() );
    node.setAttribute( "duration",  getDuration() );
    node.setAttribute( "color_r",   col.r );
    node.setAttribute( "color_g",   col.g );
    node.setAttribute( "color_b",   col.b );
    node.setAttribute( "color_a",   col.a );
    
    return node;
}


float QTimelineCue::getTimeNorm()
{
    return ( QTimeline::getPtr()->getTime() - getStartTime() ) / ( getEndTime() - getStartTime() );
}

