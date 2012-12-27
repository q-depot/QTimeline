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

#define TIMELINE_MENU_ITEM_HEIGHT   22
#define TIMELINE_MENU_TEXT_INDENT   4


class QTimelineMenuItem
{
    
    friend class QTimelineMenu;
    
public:
    
    template<typename T>
    QTimelineMenuItem( std::string name, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItem*) ) : mName(name), mMeta(meta), mIsActive(true)
    {
        mCallback.registerCb( std::bind1st( std::mem_fun( callback ), obj ) );
    }
    
    QTimelineMenuItem( std::string name ) : mName(name), mMeta(""), mIsActive(false) { }
    
    ~QTimelineMenuItem() {}
    
    std::string getName() { return mName; }

    std::string getMeta() { return mMeta; }
    
    bool isActive() { return mIsActive; }
    
    
private:
    
    std::string     mName;
    std::string     mMeta;
    bool            mIsActive;
    
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
        mItems.clear();
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

            if ( mItems[idx]->isActive() )
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
        mItems.push_back( new QTimelineMenuItem( name, meta, obj, callback ) );
        
        setWidth();
    }

    void addItem( std::string name )
    {
        mItems.push_back( new QTimelineMenuItem( name ) );

        setWidth();
    }

    bool isVisible() { return mIsVisible; }
    
    std::string getName() { return mName; }
    
    void setName( std::string name ) { mName = name; }
    
    void init( std::string name = "" )
    {
        clear();
        mName = name;
        
        addItem( mName );
    }
    
private:        // only QTimeline can render the menu
    
    void setWidth()
    {
        int width   = mFont->measureString( mName ).x;
        mWidth      = std::max( 30 + width + TIMELINE_MENU_TEXT_INDENT * 2, mWidth );
    }
    
    void render()
    {
        if ( !mIsVisible )
            return;
        
        mRect = ci::Rectf( mPos, ci::Vec2f( mPos.x + mWidth, mPos.y + TIMELINE_MENU_ITEM_HEIGHT * mItems.size() ) );
        ci::Rectf itemRect( mRect.getUpperLeft(), mRect.getUpperLeft() + ci::Vec2f( mWidth, TIMELINE_MENU_ITEM_HEIGHT ) );
        
        glBegin( GL_QUADS );
        
        ci::gl::color( ci::ColorA( 0.15f, 0.15f, 0.15f, 1.0f ) );
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        ci::gl::vertex( mRect.getLowerLeft() );
    
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y1 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y1 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y1 + 1 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y1 + 1 ) );
    
        glEnd();
        
        for( size_t k=0; k < mItems.size(); k++ )
        {
            glBegin( GL_QUADS );
            
            if( mMouseOnItem == mItems[k] )
            {
                ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
                ci::gl::vertex( itemRect.getUpperLeft() );
                ci::gl::vertex( itemRect.getUpperRight() );
                ci::gl::vertex( itemRect.getLowerRight() );
                ci::gl::vertex( itemRect.getLowerLeft() );
            }
            
            ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
            ci::gl::vertex( ci::Vec2f( itemRect.x1, itemRect.y2 ) );
            ci::gl::vertex( ci::Vec2f( itemRect.x2, itemRect.y2 ) );
            ci::gl::vertex( ci::Vec2f( itemRect.x2, itemRect.y2 - 1 ) );
            ci::gl::vertex( ci::Vec2f( itemRect.x1, itemRect.y2 - 1 ) );
            
            glEnd();
            
            if ( mItems[k]->isActive() )
                ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
            else
                ci::gl::color( ci::ColorA( 0.0f, 1.0f, 1.0f, 1.0f ) );
            
            mFont->drawString( mItems[k]->mName, itemRect.getUpperLeft() + ci::Vec2f( TIMELINE_MENU_TEXT_INDENT, 14 ) );
            
            itemRect.offset( ci::Vec2f( 0, TIMELINE_MENU_ITEM_HEIGHT ) );
        }
        
//        mRect = ci::Rectf( upperLeft, r.getUpperRight() );
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
