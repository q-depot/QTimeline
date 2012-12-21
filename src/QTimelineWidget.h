/*
 *  QTimelineWidget.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_WIDGET
#define QTIMELINE_WIDGET

#define TIMELINE_WIDGET_PADDING 1

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Xml.h"
#include "QTimelineMenu.h"


typedef std::shared_ptr<class QTimelineModuleItem>		QTimelineModuleItemRef;

typedef std::shared_ptr<class QTimelineWidget>          QTimelineWidgetRef;

class QTimelineWidget
{
    
public:
    
    QTimelineWidget( std::string name = "" )
    {
        mFont           = ci::gl::TextureFont::create( ci::Font( "Helvetica", 12 ) );
        mMenu           = new QTimelineMenu();
        
        setName( name );
    }
    
    ~QTimelineWidget()
    {
        if ( mMenu )
            delete mMenu;
    }
    
    bool contains( ci::Vec2f pos )
    {
        return mRect.contains( pos );
    }
    
    virtual void setRect( ci::Rectf rect )
    {
        mRect               = rect;
        mRectPaddedHeight   = mRect.inflated( ci::Vec2f( 0, TIMELINE_WIDGET_PADDING ) );
    }
    
    std::string getName() { return mName; }
    
	void        setName( std::string name )
    {
        mName           = name;
        mNameStrSize    = mFont->measureString( mName );

        updateLabel();
    }
    
    std::string getLabel() { return mLabel; }
	
    ci::Rectf   getRect() { return mRect; }
    
    ci::Rectf   getRectPaddedHeight() { return mRectPaddedHeight; }
    
    float	getHeight() { return mRect.getHeight(); }
    
	ci::ColorA  getBgColor() { return mBgColor; }
    void        setBgColor( ci::ColorA col )        { mBgColor      = col; }
    
    ci::ColorA  getBgOverColor() { return mBgOverColor; }
    void        setBgOverColor( ci::ColorA col )    { mBgOverColor  = col; }
    
	ci::ColorA  getFgColor() { return mFgColor; }
    void        setFgColor( ci::ColorA col )        { mFgColor      = col; }
    
    ci::ColorA  getTextColor() { return mTextColor; }
    void        setTextColor( ci::ColorA col )      { mTextColor    = col; }
    
    virtual bool mouseMove( ci::app::MouseEvent event ) { return false; }
    virtual bool mouseDown( ci::app::MouseEvent event ) { return false; }
    virtual bool mouseUp( ci::app::MouseEvent event )   { return false; }
    virtual bool mouseDrag( ci::app::MouseEvent event ) { return false; }
    virtual bool keyDown( ci::app::KeyEvent event ) { return false; }
    
    virtual ci::XmlTree getXmlNode() { return ci::XmlTree(); }
    
    virtual void loadXmlNode( ci::XmlTree node ) {}
	
protected:
    
    virtual void updateLabel()
    {
        mLabel = mName;
        
        if ( mLabel == "" )
            return;
        
        int charsN = mLabel.size() * ( mRect.getWidth() ) / mNameStrSize.x;

        if ( charsN < mLabel.size() )
            mLabel.resize( charsN );
        
        mLabelStrSize = mFont->measureString( mLabel );
    }
    
    virtual void menuEventHandler( QTimelineMenuItem* item ) {}
    
    virtual void initMenu() {}
    
    
protected:
    
    std::string     mName;
    ci::Vec2f       mNameStrSize;
    
    std::string     mLabel;
    ci::Vec2f       mLabelStrSize;
    
    ci::Rectf       mRect;
    ci::Rectf       mRectPaddedHeight;
    
    ci::ColorA      mBgColor;
    ci::ColorA      mBgOverColor;
    ci::ColorA      mFgColor;
    ci::ColorA      mTextColor;
    
    ci::gl::TextureFontRef	mFont;
    
    ci::Vec2f       mMouseDownPos;
    ci::Vec2f       mMousePrevPos;
    
    float           mMouseDownStartTime;
    float           mMouseDownEndTime;
    
    QTimelineMenu                   *mMenu;
    
    std::vector<QTimelineWidgetRef> mChildren;
};


#endif