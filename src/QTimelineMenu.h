/*
 *  QTimelineMenu.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
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
#define TIMELINE_MENU_TEXT_INDENT   5

typedef std::shared_ptr<class QTimelineMenuItem>    QTimelineMenuItemRef;


/*--- generic Item ---*/

class QTimelineMenuItem : public std::enable_shared_from_this<QTimelineMenuItem>
{
    
    friend class QTimelineMenu;
    
public:
    
    QTimelineMenuItem( std::string name = "", std::string meta = "", bool isActive = false ) : mName(name), mMeta(meta), mIsActive(isActive)
    {
        init();
    }
    
    template<typename T>
    QTimelineMenuItem( std::string name, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItemRef) ) : mName(name), mMeta(meta), mIsActive(true)
    {
        mCallback.registerCb( std::bind1st( std::mem_fun( callback ), obj ) );

        init();
    }

    virtual void init()
    {
        mHeight = 0.0f;
        
        mFont   = ci::gl::TextureFont::create( ci::Font( "Helvetica", 12 ) );
    }
    
    virtual ~QTimelineMenuItem() {}
    
    virtual void call()
    {
        if ( !mCallback.empty() )
            mCallback.call( shared_from_this() );
    }
    
    virtual ci::Rectf render( ci::Rectf r, bool mouseOver, bool isOnFocuse = false ) { return r; }
    
    virtual bool contains( ci::Vec2f pos ) { return mRect.contains( pos ); }
    
    std::string getName() { return mName; }
    
    void setName( std::string name ) { mName = name; }
    
    std::string getMeta() { return mMeta; }
    
    bool isActive() { return mIsActive; }
    
    float getHeight() { return mHeight; }
    
    virtual bool mouseMove( ci::app::MouseEvent event ) { return false; }
    
    virtual bool mouseDown( ci::app::MouseEvent event ) { return false; }
    
    virtual bool mouseDrag( ci::app::MouseEvent event ) { return false; }
    
    virtual bool keyDown( ci::app::KeyEvent event ) { return false; }
    
    virtual void open() {}
    
    virtual void close() {}
    
protected:
    
    float                       mHeight;
    ci::gl::TextureFontRef      mFont;
    ci::Rectf                   mRect;
    
private:
    
    std::string     mName;
    std::string     mMeta;
    bool            mIsActive;
    
    ci::CallbackMgr<void (QTimelineMenuItemRef)>  mCallback;
    
};

/*--- Button ---*/

class QTimelineMenuButton : public QTimelineMenuItem
{
    
public:
    
    template<typename T>
    QTimelineMenuButton( std::string name, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItemRef) ) : QTimelineMenuItem( name, meta, obj, callback )
    {
        mHeight = TIMELINE_MENU_ITEM_HEIGHT;
    }
    
    ~QTimelineMenuButton() {}
    
    ci::Rectf render( ci::Rectf r, bool mouseOver, bool isOnFocuse = false )
    {
        mRect = ci::Rectf( r.getLowerLeft(), r.getLowerRight() + ci::Vec2f( 0, mHeight ) );
        
        glBegin( GL_QUADS );

        // background
        if( mouseOver )
            ci::gl::color( ci::ColorA( 0.2f, 0.2f, 0.2f, 1.0f ) );
        else
            ci::gl::color( ci::ColorA( 0.15f, 0.15f, 0.15f, 1.0f ) );
        
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        ci::gl::vertex( mRect.getLowerLeft() );
        
        // border bottom
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 - 1 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 - 1 ) );
        
        glEnd();
        
        // label
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
        mFont->drawString( getName(), mRect.getUpperLeft() + ci::Vec2f( TIMELINE_MENU_TEXT_INDENT, 14 ) );
        
        return mRect;
    }

    bool mouseDown( ci::app::MouseEvent event )
    {
        call();
        return true;
    }

};

/*--- Label ---*/

class QTimelineMenuLabel : public QTimelineMenuItem
{
    
public:
    
    QTimelineMenuLabel( std::string name ) : QTimelineMenuItem( name )
    {
        mHeight = TIMELINE_MENU_ITEM_HEIGHT;
    }
    
    ~QTimelineMenuLabel() {}
    
    ci::Rectf render( ci::Rectf r, bool mouseOver, bool isOnFocuse = false )
    {
        mRect = ci::Rectf( r.getLowerLeft(), r.getLowerRight() + ci::Vec2f( 0, mHeight ) );
        
        glBegin( GL_QUADS );
        
        // background
        ci::gl::color( ci::ColorA( 0.18f, 0.18f, 0.18f, 1.0f ) );
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        ci::gl::vertex( mRect.getLowerLeft() );
        
        // border bottom
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 - 1 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 - 1 ) );
        
        glEnd();
        
        // label
        ci::gl::color( ci::ColorA( 0.0f, 1.0f, 1.0f, 1.0f ) );
        mFont->drawString( getName(), mRect.getUpperLeft() + ci::Vec2f( TIMELINE_MENU_TEXT_INDENT, 14 ) );
        
        return mRect;
    }
};

/*--- Separator ---*/

class QTimelineMenuSeparator : public QTimelineMenuItem
{
    
public:
    
    QTimelineMenuSeparator() : QTimelineMenuItem( "", "separator" )
    {
        mHeight = 3;
    }
    
    ~QTimelineMenuSeparator() {}
    
    ci::Rectf render( ci::Rectf r, bool mouseOver, bool isOnFocuse = false )
    {
        mRect = ci::Rectf( r.getLowerLeft(), r.getLowerRight() + ci::Vec2f( 0, mHeight ) );
        
        glBegin( GL_QUADS );
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.8f ) );
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        ci::gl::vertex( mRect.getLowerLeft() );
        glEnd();
        
        return mRect;
    }
};


/*--- Color palette ---*/

class QTimelineMenuColorPalette : public QTimelineMenuItem
{
    
public:
    
    template<typename T>
    QTimelineMenuColorPalette( T *obj, void (T::*callback)(QTimelineMenuItemRef) ) : QTimelineMenuItem( "", "color_palette", obj, callback )
    {
        mHeight = TIMELINE_MENU_ITEM_HEIGHT;
        
        //        mColors.push_back( ci::Color( 0.86f, 0.18f, 0.11f ) );
        //        mColors.push_back( ci::Color( 1.0f, 0.34f, 0.0f ) );
        //        mColors.push_back( ci::Color( 0.86f, 0.62f, 0.0f ) );
        //        mColors.push_back( ci::Color( 0.0f, 0.65f, 0.58f ) );
        //        mColors.push_back( ci::Color( 0.45f, 0.60f, 0.0f ) );
        //        mColors.push_back( ci::Color( 0.5f, 0.5f, 0.5f ) );
        
    }
    
    ~QTimelineMenuColorPalette() {}
    
    ci::Rectf render( ci::Rectf r, bool mouseOver, bool isOnFocuse = false )
    {
        mRect = ci::Rectf( r.getLowerLeft(), r.getLowerRight() + ci::Vec2f( 0, mHeight ) );
        
        glBegin( GL_QUADS );
        
        // background
        ci::gl::color( ci::ColorA( 0.15f, 0.15f, 0.15f, 1.0f ) );
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        ci::gl::vertex( mRect.getLowerLeft() );
        
        // colors
        float w = mRect.getWidth() / QTimelineMenuColorPalette::mColors.size();
        
        r = ci::Rectf( mRect.getUpperLeft(), mRect.getLowerLeft() + ci::Vec2f( w, 0 ) );
        r.inflate( ci::Vec2f( -2.0f, -3.0f ) );
        
        for( size_t k=0; k < QTimelineMenuColorPalette::mColors.size(); k++ )
        {
            ci::gl::color( QTimelineMenuColorPalette::mColors[k] );
            ci::gl::vertex( r.getUpperLeft() );
            ci::gl::vertex( r.getUpperRight() );
            ci::gl::vertex( r.getLowerRight() );
            ci::gl::vertex( r.getLowerLeft() );
            
            r.offset( ci::Vec2f( w, 0 ) );
        }
        
        // border bottom
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 - 1 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 - 1 ) );
        
        glEnd();
        
        return mRect;
    }
    
    ci::Color getColor() { return mSelectedColor; }
    
    bool contains( ci::Vec2f pos )
    {
        if ( mRect.contains( pos ) )
        {
            pos         -= mRect.getUpperLeft();
            float   w   = mRect.getWidth() / QTimelineMenuColorPalette::mColors.size();
            int     idx = ci::math<int>::clamp( pos.x / w, 0, QTimelineMenuColorPalette::mColors.size() - 1 );
            
            mSelectedColor = mColors[idx];
            
            return true;
        }
        return false;
    }
    
    bool mouseDown( ci::app::MouseEvent event )
    {
        call();
        return true;
    }

public:
    
    static std::vector<ci::ColorA> mColors;
    
private:
    
    ci::Color               mSelectedColor;
    
};


/*---------------------*/


/*--- Text box ---*/

class QTimelineMenuTextBox : public QTimelineMenuItem
{
    
public:
    
    template<typename T>
    QTimelineMenuTextBox( std::string text, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItemRef) ) : QTimelineMenuItem( text, meta, obj, callback )
    {
        mHeight = TIMELINE_MENU_ITEM_HEIGHT;
    }
    
    ~QTimelineMenuTextBox() {}
    
    ci::Rectf render( ci::Rectf r, bool mouseOver, bool isOnFocus = false )
    {
        mRect                   = ci::Rectf( r.getLowerLeft(), r.getLowerRight() + ci::Vec2f( 0, mHeight ) );
        ci::Vec2f   textOffset  = ci::Vec2f( TIMELINE_MENU_TEXT_INDENT, 14 );
        
        glBegin( GL_QUADS );
        
        // background
        if( mouseOver )
            ci::gl::color( ci::ColorA( 0.2f, 0.2f, 0.2f, 1.0f ) );
        else
            ci::gl::color( ci::ColorA( 0.15f, 0.15f, 0.15f, 1.0f ) );
        
        // bg
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        ci::gl::vertex( mRect.getLowerLeft() );
        
        // container
        ci::gl::color( ci::ColorA( 0.0f, 0.0f, 0.0f, 0.3f ) );
        ci::Rectf boxRect = mRect.inflated( ci::Vec2f( -3.0f, -3.0f ) );
        ci::gl::vertex( boxRect.getUpperLeft() );
        ci::gl::vertex( boxRect.getUpperRight() );
        ci::gl::vertex( boxRect.getLowerRight() );
        ci::gl::vertex( boxRect.getLowerLeft() );
        
        // border bottom
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x2, mRect.y2 - 1 ) );
        ci::gl::vertex( ci::Vec2f( mRect.x1, mRect.y2 - 1 ) );
        
        // cursor
        if ( isOnFocus )
        {
            int alpha = fmod( ci::app::getElapsedSeconds(), 1.0f ) + 0.5f;
            ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, alpha  ) );
            ci::gl::vertex( ci::Vec2f( mCursorPosX,     mRect.y1 + 4 ) );
            ci::gl::vertex( ci::Vec2f( mCursorPosX + 1, mRect.y1 + 4 ) );
            ci::gl::vertex( ci::Vec2f( mCursorPosX + 1, mRect.y1 + 16 ) );
            ci::gl::vertex( ci::Vec2f( mCursorPosX,     mRect.y1 + 16 ) );
        }
        
        glEnd();
        
        // label
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
        mFont->drawString( mLabel, mRect.getUpperLeft() + textOffset );
        
        return mRect;
    }
    
    bool mouseDown( ci::app::MouseEvent event )
    {
        float       x           = event.getPos().x  - mRect.x1 - TIMELINE_MENU_TEXT_INDENT;
        std::string text        = mLabel;
        ci::Vec2f   textSize    = mFont->measureString( text );
        mCursorIdx              = text.size();
        
        while( textSize.x > x )
        {
            text        = text.erase( text.size() - 1 );
            textSize    = mFont->measureString( text );

            mCursorIdx--;
        }
        
        setCursorPos( mCursorIdx );

        return false;
    }
    
    bool keyDown( ci::app::KeyEvent event )
    {
        int     code    = event.getCode();
        char    c       = event.getChar();
        
        if ( code == ci::app::KeyEvent::KEY_RETURN )
        {
            call();
            return true;
        }
        
        else if ( code == ci::app::KeyEvent::KEY_BACKSPACE )
        {
            if ( mCursorIdx > 0 )
            {
                mLabel = mLabel.erase( mCursorIdx - 1, 1 );
                mCursorIdx -= 1;
                setCursorPos( mCursorIdx );
            }
            return true;
        }
        
        else if ( code == ci::app::KeyEvent::KEY_LEFT )
        {
            mCursorIdx = std::max( mCursorIdx - 1, 0 );
            setCursorPos( mCursorIdx );
            return true;
        }
        
        else if ( code == ci::app::KeyEvent::KEY_RIGHT )
        {
            int maxSize = mLabel.size();
            mCursorIdx = std::min( mCursorIdx + 1, maxSize );
            setCursorPos( mCursorIdx );
            return true;
        }
        
        else
        {
            mLabel = mLabel.insert( mCursorIdx, 1, c );
            mCursorIdx += 1;
            setCursorPos( mCursorIdx );
            return true;
        }
        
        return false;
    }
    
    void open() { mLabel = getName(); }
    
private:
    
    void setCursorPos( int idx )
    {
        std::string text = mLabel;
        text = text.erase( idx, text.size() - idx );
        mCursorPosX = mFont->measureString( text ).x + mRect.x1 + TIMELINE_MENU_TEXT_INDENT;
    }
    
private:
    
    int             mCursorIdx;
    float           mCursorPosX;
    std::string     mLabel;
    
};

/*---------------------*/


/*--- QTimelineMenu ---*/


class QTimelineMenu
{

    friend class QTimeline;
    
public:
    
    
    QTimelineMenu()
    {
        mIsVisible      = false;

        calculateSize();
    }
    
    ~QTimelineMenu()
    {
        clear();
    }
    
    void clear()
    {
        mMouseOnItem.reset();
        mItemOnFocus.reset();
        
        mItems.clear();
    }
    
    void open( ci::Vec2f pos  )
    {
        for( size_t k=0; k < mItems.size(); k++ )
            mItems[k]->open();
        
        mIsVisible  = true;
        
        if ( pos.x + mSize.x > ci::app::getWindowWidth() )
            pos.x -= mSize.x;
        
        if ( pos.y + mSize.y > ci::app::getWindowHeight() )
            pos.y -= mSize.y;

        mPos        = pos;
        
        mItemOnFocus.reset();
    }
    
    void close() { mIsVisible = false; }
    
    bool mouseMove( ci::app::MouseEvent event )
    {
        mMouseOnItem.reset();
        
        for( size_t k=0; k < mItems.size(); k++ )
        {
            if ( mItems[k]->contains( event.getPos() ) )
            {
                mMouseOnItem = mItems[k];
                return true;
            }
        }
        
        if ( mItemOnFocus )
        {
            mItemOnFocus->mouseMove( event );
            return true;
        }
        
        return false;
    }

    bool mouseDown( ci::app::MouseEvent event )
    {
        mItemOnFocus = mMouseOnItem;
        
        if ( mItemOnFocus )
        {
            mItemOnFocus->mouseDown( event );
            return true;
        }
        
        return false;
    }
    
    bool mouseDrag( ci::app::MouseEvent event )
    {
        if ( mItemOnFocus )
        {
            mItemOnFocus->mouseDrag( event );
            return true;
        }
        
//        if ( mMouseOnItem )
//            return true;
//        
        return false;
    }

    bool keyDown( ci::app::KeyEvent event )
    {
        if ( mItemOnFocus )
        {
            mItemOnFocus->keyDown( event );
            return true;
        }
        
        return false;
    }
    
    bool isVisible() { return mIsVisible; }
    
    std::string getName() { return mName; }
    
    void setName( std::string name ) { mName = name; }
    
    void init( std::string name = "" )
    {
        clear();
        
        mName = name;
        
        addLabel( mName );
    }
    
    // add item functions
    // ------------------
    
    void addLabel( std::string name )
    {
        addItem( QTimelineMenuItemRef( new QTimelineMenuLabel(name) ) );
    }
    
    template<typename T>
    void addButton( std::string name, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItemRef) )
    {
        addItem( QTimelineMenuItemRef( new QTimelineMenuButton( name, meta, obj, callback ) ) );
    }
    
    void addSeparator()
    {
        addItem( QTimelineMenuItemRef( new QTimelineMenuSeparator() ) );
    }
    
    template<typename T>
    void addColorPalette( T *obj, void (T::*callback)(QTimelineMenuItemRef) )
    {
        addItem( QTimelineMenuItemRef( new QTimelineMenuColorPalette( obj, callback ) ) );
    }
    
    template<typename T>
    void addTextBox( std::string text, std::string meta, T *obj, void (T::*callback)(QTimelineMenuItemRef) )
    {
        addItem( QTimelineMenuItemRef( new QTimelineMenuTextBox( text, meta, obj, callback ) ) );
    }
    
private:
    
    void addItem( QTimelineMenuItemRef item )
    {
        mItems.push_back( item );
        calculateSize();
    }
    
    void calculateSize()
    {
        mSize = ci::Vec2f::zero();
        
        for( size_t k=0; k < mItems.size(); k++ )
            mSize.y += mItems[k]->getHeight();
        
        mSize.x = 150;
//        int width   = mFont->measureString( mName ).x;
//        mWidth      = std::max( 30 + width + TIMELINE_MENU_TEXT_INDENT * 2, mWidth );
    }
    
    void render()
    {
        if ( !mIsVisible )
            return;
        
        ci::Rectf r( mPos, mPos + ci::Vec2f( mSize.x, 0 ) );
        
        bool mouseOver, isOnFocus;
        
        for( size_t k=0; k < mItems.size(); k++ )
        {
            mouseOver = ( mItems[k] == mMouseOnItem ) ? true : false;
            isOnFocus = ( mItems[k] == mItemOnFocus ) ? true : false;
            r = mItems[k]->render( r, mouseOver, mItemOnFocus );
        }
        
        // border top
        glBegin( GL_QUADS );
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.2f ) );
        ci::gl::vertex( mPos );
        ci::gl::vertex( mPos + ci::Vec2f( mSize.x,  + 0 ) );
        ci::gl::vertex( mPos + ci::Vec2f( mSize.x,  - 1 ) );
        ci::gl::vertex( mPos + ci::Vec2f( 0,        - 1 ) );
        glEnd();
    }

private:
    
    ci::Vec2f                           mPos;
    
    ci::Rectf                           mRect;
    
    std::string                         mName;
    std::vector<QTimelineMenuItemRef>   mItems;
    ci::Vec2f                           mSize;
    QTimelineMenuItemRef                mMouseOnItem;
    QTimelineMenuItemRef                mItemOnFocus;
    bool                                mIsVisible;
};


#endif
