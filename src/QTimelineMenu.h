/*
 *  QTimelineMenu.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef QTIMELINE_MENU
#define QTIMELINE_MENU

#pragma once

#include "cinder/gl/TextureFont.h"
#include "cinder/Function.h"

#define TIMELINE_MENU_ITEM_HEIGHT   20
#define TIMELINE_MENU_TEXT_INDENT   3


class QTimelineMenuItem
{
    
    friend class QTimelineMenu;
    
public:
    
    template<typename T>
    QTimelineMenuItem( std::string name, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItem*) ) : mName(name), mMeta(meta)
    {
        mCallback.registerCb( std::bind1st( std::mem_fun( callback ), obj ) );
    }
    
    ~QTimelineMenuItem() {}
    
    std::string getName() { return mName; }

    std::string getMeta() { return mMeta; }
    
private:
    
    std::string                                 mName;
    std::string                                 mMeta;
    ci::CallbackMgr<void (QTimelineMenuItem*)>  mCallback;
};


class QTimelineMenu
{

    friend class QTimeline;
    
public:
    
    
    QTimelineMenu()
    {
        mWidth          = 0;
        mMouseOnItem    = NULL;
        mFont           = ci::gl::TextureFont::create( ci::Font( "Helvetica", 12 ) );
        mIsVisible      = false;
    }
    
    ~QTimelineMenu()
    {
        clear();
    }
    
    void clear()
    {
        for( size_t k=0; k < mItems.size(); k++ )
            delete mItems[k];
    }
    
    void open( ci::Vec2f pos  )
    {
        mIsVisible  = true;
        mPos        = pos;
    }
    
    void close() { mIsVisible = false; }
    
    bool mouseMove( ci::app::MouseEvent event )
    {
        mMouseOnItem = NULL;
        
        if ( mRect.contains( event.getPos() ) )
        {
            int idx = mItems.size() * ( event.getPos().y - mRect.y1 ) / mRect.getHeight();
            mMouseOnItem = mItems[idx];
            return true;
        }
        
        return false;
    }

    bool mouseDown( ci::app::MouseEvent event )
    {
        if ( mMouseOnItem )
        {
            mMouseOnItem->mCallback.call( mMouseOnItem );
            return true;
        }
        
        return false;
    }
    
    template<typename T>
    void addItem( std::string name, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItem*) )
    {
        QTimelineMenuItem *item = new QTimelineMenuItem( name, meta, obj, callback );
        mItems.push_back( item );
        
        int width   = mFont->measureString( name ).x;
        mWidth      = std::max( width + TIMELINE_MENU_TEXT_INDENT * 2, mWidth );
    }
    
    bool isVisible() { return mIsVisible; }
    
    std::string getName() { return mName; }
    
    void setName( std::string name ) { mName = name; }
    
    void init( std::string name = "" )
    {
        clear();
        mName = name;
    }
    
private:        // only QTimeline can render the menu
    
    void render()
    {
        if ( !mIsVisible )
            return;
        
        ci::Vec2f upperLeft = mPos;//( 100, 100 );
        ci::Vec2f lowerRight( upperLeft.x + mWidth, upperLeft.y + TIMELINE_MENU_ITEM_HEIGHT * mItems.size() );
        
        ci::gl::color( ci::ColorA( 0.2f, 0.2f, 0.2f, 1.0f ) );
        ci::gl::drawSolidRect( ci::Rectf( upperLeft, lowerRight ) );
        
        ci::Rectf r( upperLeft, upperLeft + ci::Vec2f( mWidth, TIMELINE_MENU_ITEM_HEIGHT ) );
        for( size_t k=0; k < mItems.size(); k++ )
        {
            if( mMouseOnItem == mItems[k] ) ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) ); else ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.1f ) );
            ci::gl::drawSolidRect( r );
            ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
            mFont->drawString( mItems[k]->mName, upperLeft + ci::Vec2f( TIMELINE_MENU_TEXT_INDENT, 13 + k * TIMELINE_MENU_ITEM_HEIGHT ) );
            ci::gl::color( ci::Color::white() );
            ci::gl::drawLine( r.getLowerLeft(), r.getLowerRight() );
            
            r.offset( ci::Vec2f( 0, TIMELINE_MENU_ITEM_HEIGHT ) );
        }
        
        mRect = ci::Rectf( upperLeft, r.getUpperRight() );
    }
    
private:
    
    ci::Vec2f                           mPos;
    
    ci::Rectf                           mRect;
    
    ci::gl::TextureFontRef              mFont;
    
    std::string                         mName;
    std::vector<QTimelineMenuItem*>     mItems;
    int                                 mWidth;
    QTimelineMenuItem                   *mMouseOnItem;
    bool                                mIsVisible;
};


#endif
