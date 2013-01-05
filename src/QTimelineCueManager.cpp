/*
 *  QTimelineCueManager.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimelineCueManager.h"
#include "QTimeline.h"

bool sortCueListHelper( QTimelineCueRef a, QTimelineCueRef b ) { return ( a->getStartTime() < b->getStartTime() ); }

using namespace ci;
using namespace ci::app;
using namespace std;


QTimelineCueManager::QTimelineCueManager( QTimeline *qTimeline ) : mQTimeline(qTimeline)
{
    initMenu();
}


void QTimelineCueManager::render()
{
    QTimelineCueRef cue;
    Rectf           cueRect;
    float           startPos, endPos;
    
    gl::color( mBgColor );
    
    glBegin( GL_QUADS );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    gl::vertex( mRect.getLowerLeft() );
    glEnd();
    
    for( size_t k=0; k < mCueList.size(); k++ )
    {
        cue     = mCueList[k];
        cueRect = cue->getRect();
        
        startPos    = mQTimeline->getPosFromTime( cue->getStartTime() );
        endPos      = mQTimeline->getPosFromTime( cue->getEndTime() );
        cue->setRect( Rectf( startPos, mRect.y1, endPos, mRect.y2 ) );
        
        cue->render();
    }
}


void QTimelineCueManager::addCue( std::string name, double startTime, double duration )
{
    mCueList.push_back( QTimelineCueRef( new QTimelineCue( mQTimeline, this, name, startTime, duration ) ) );
    
    sort( mCueList.begin(), mCueList.end(), sortCueListHelper );
}


void QTimelineCueManager::deleteCue( QTimelineCue *cue )
{
    for( size_t k=0; k < mCueList.size(); k++ )
        if ( mCueList[k].get() == cue )
            mCueList.erase( mCueList.begin() + k );
    
    mCurrentCue.reset();
}


bool QTimelineCueManager::mouseMove( ci::app::MouseEvent event )
{
    for( size_t k=0; k < mCueList.size(); k++ )
        mCueList[k]->mouseMove( event );
            
    mMousePrevPos = event.getPos();
    
    return false;
}


bool QTimelineCueManager::mouseDown( ci::app::MouseEvent event )
{
    bool mouseOnCue = false;
  
    for( size_t k=0; k < mCueList.size(); k++ )
        if( mCueList[k]->mouseDown( event ) )
            mouseOnCue = true;

    if ( !mouseOnCue && event.isRightDown() && mRect.contains( event.getPos() ) )
        mQTimeline->openMenu( mMenu, event.getPos() );

    mMouseDownPos = event.getPos();
    
    return false;
}


bool QTimelineCueManager::mouseUp( ci::app::MouseEvent event )
{
    for( size_t k=0; k < mCueList.size(); k++ )
        mCueList[k]->mouseUp( event );
    
    return false;
}


bool QTimelineCueManager::mouseDrag( ci::app::MouseEvent event )
{
    for( size_t k=0; k < mCueList.size(); k++ )
        mCueList[k]->mouseDrag( event );
    
    mMousePrevPos = event.getPos();
    
    return false;
}


void QTimelineCueManager::menuEventHandler( QTimelineMenuItemRef item )
{
    console() << "create new cue" << endl;
    
    addCue( "CUE " + toString( mCueList.size() ), mQTimeline->getTimeFromPos( mMouseDownPos.x ), 1.0f );
    mQTimeline->closeMenu( mMenu );
}


void QTimelineCueManager::initMenu()
{
    mMenu->init( "CUE MANAGER MENU" );
    
    mMenu->addButton( "New cue", "", this, &QTimelineCueManager::menuEventHandler );
}


bool QTimelineCueManager::playCue( int cueN )
{
    if ( mCueList.empty() )
        return false;
    

    if ( cueN >= 0 )                                        // select a specific cue within the range
    {
        if ( cueN >= mCueList.size() )
            return false;
        
        mCurrentCue = mCueList[cueN];
        return true;                                                // select cue
    }
    
                                                            // or select the next cue
    if ( !mCurrentCue )
    {
        mCurrentCue = mCueList[0];
        return true;                                                // select cue
    }
    
    if ( isTimeOnCue() )
        return false;
    
    for( size_t k=0; k < mCueList.size(); k++ )
    {
        if ( mCueList[k] == mCurrentCue )
        {
            mCurrentCue = mCueList[ (k+1) % mCueList.size() ];
            mQTimeline->setTime( mCurrentCue->getStartTime() );
            return true;                                            // select cue
        }
    }
    
    return false;
}


bool QTimelineCueManager::isTimeOnCue()
{
    double time = mQTimeline->getTime();
    if ( time >= mCurrentCue->getStartTime() && time < mCurrentCue->getEndTime() )
        return true;
    
    return false;
}


XmlTree QTimelineCueManager::getXmlNode()
{
    XmlTree node( "cueList", "" );
    
    for( size_t k=0; k < mCueList.size(); k++ )
        node.push_back( mCueList[k]->getXmlNode() );
    
    return node;
}


void QTimelineCueManager::loadXmlNode( XmlTree node )
{
    string  name;
    float   startTime, duration;
    ColorA  col;
    
    for( XmlTree::Iter nodeIt = node.begin("cue"); nodeIt != node.end(); ++nodeIt )
    {
        name        = nodeIt->getAttributeValue<string>( "name" );
        startTime   = nodeIt->getAttributeValue<float>( "startTime" );
        duration    = nodeIt->getAttributeValue<float>( "duration" );
        col.r       = nodeIt->getAttributeValue<float>( "color_r" );
        col.g       = nodeIt->getAttributeValue<float>( "color_g" );
        col.b       = nodeIt->getAttributeValue<float>( "color_b" );
        col.a       = nodeIt->getAttributeValue<float>( "color_a" );
        
        QTimelineCueRef ref = QTimelineCueRef( new QTimelineCue( mQTimeline, this, name, startTime, duration ) );
        ref->setColor( col );
        
        mCueList.push_back( ref );
    }
    
    sort( mCueList.begin(), mCueList.end(), sortCueListHelper );
}

