/*
 *  QTimelineWidgetWithHandles.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_WIDGET_WITH_HANDLES
#define QTIMELINE_WIDGET_WITH_HANDLES

#pragma once

#include "QTimelineWidget.h"

#define TIMELINE_MODULE_HANDLE_WIDTH    7


class QTimelineWidgetWithHandles : public QTimelineWidget
{

public:
    
    QTimelineWidgetWithHandles( std::string name = "" ) : QTimelineWidget(name)
    {
        mSelectedHandleType = NO_HANDLE;
        mMouseOnHandleType  = NO_HANDLE;
    }
    
    
    ~QTimelineWidgetWithHandles() {}

    
    virtual void setRect( ci::Rectf rect )
    {
        QTimelineWidget::setRect( rect );
        
        mLeftHandleRect     = ci::Rectf( mRect.getUpperLeft(), mRect.getLowerLeft() + ci::Vec2f( TIMELINE_MODULE_HANDLE_WIDTH, 0) );
        mRightHandleRect    = ci::Rectf( mRect.getUpperRight() - ci::Vec2f( TIMELINE_MODULE_HANDLE_WIDTH, 0), mRect.getLowerRight() );
    }
    
    void renderHandles()
    {
        if ( mMouseOnHandleType == LEFT_HANDLE ) ci::gl::color( mHandleOverColor ); else ci::gl::color( mHandleColor );
        ci::gl::drawSolidRect( mLeftHandleRect );
        
        if ( mMouseOnHandleType == RIGHT_HANDLE ) ci::gl::color( mHandleOverColor ); else ci::gl::color( mHandleColor );
        ci::gl::drawSolidRect( mRightHandleRect );
    }
    
    bool handlesMouseMove( ci::Vec2f pos )
    {
        if ( mLeftHandleRect.contains( pos ) )
        {
            mMouseOnHandleType = LEFT_HANDLE;
            return true;
        }
        
        else if ( mRightHandleRect.contains( pos ) )
        {
            mMouseOnHandleType = RIGHT_HANDLE;
            return true;
        }
        
        return false;
    }

    bool handlesMouseDown()
    {
        mSelectedHandleType = mMouseOnHandleType;
        mLeftHandleRect     = ci::Rectf( mRect.getUpperLeft(), mRect.getLowerLeft() + ci::Vec2f( TIMELINE_MODULE_HANDLE_WIDTH, 0) );
        mRightHandleRect    = ci::Rectf( mRect.getUpperRight() - ci::Vec2f( TIMELINE_MODULE_HANDLE_WIDTH, 0), mRect.getLowerRight() );

        return ( mSelectedHandleType != NO_HANDLE ) ? false : true;
    }
    
    virtual bool dragHandles( ci::app::MouseEvent event ) { return false; }
    
    virtual void dragWidget( ci::app::MouseEvent event ) {}
    
    void setHandleColor( ci::ColorA col ) { mHandleColor = col; }
    
    void setHandleOverColor( ci::ColorA col ) { mHandleOverColor = col; }
    
    ci::ColorA getHandleColor() { return mHandleColor; }
    
    ci::ColorA getHandleOverColor() { return mHandleOverColor; }
    
protected:
    
    enum HandleType {
        LEFT_HANDLE,
        RIGHT_HANDLE,
        NO_HANDLE
    };
    
    ci::Rectf                       mLeftHandleRect, mRightHandleRect;
    
    ci::ColorA                      mHandleColor;
    ci::ColorA                      mHandleOverColor;
    HandleType                      mSelectedHandleType;
    HandleType                      mMouseOnHandleType;

};


#endif