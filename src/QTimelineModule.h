/*
 *  QTimelineModule.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_MODULE
#define QTIMELINE_MODULE

#pragma once

#include "QTimelineItem.h"

typedef std::shared_ptr<class QTimelineModule>    QTimelineModuleRef;


class QTimelineModule
{
    
public:
    
    QTimelineModule( std::string type ) : mType(type), mMarkedForRemoval(false) {}
    
    QTimelineModule( QTimelineItemRef item, std::string type ) : mItemRef(item), mType(type), mMarkedForRemoval(false) {}
    
    virtual ~QTimelineModule()
    {
        mItemRef.reset();
    }
    
    virtual void update() {}
    
    virtual void render() {}
    
    virtual void init() {}
  
    virtual void activeChanged(bool active) {}
  
    void setItemRef( QTimelineItemRef ref ) { mItemRef = ref; }
    
    QTimelineItemRef getItemRef() { return mItemRef->thisRef(); }

    size_t getNumParams() { return ( mItemRef ) ? mItemRef->getNumParams() : 0; }
    
    std::vector<QTimelineParamRef> getParams()
    {
        std::vector<QTimelineParamRef> emptyVec;
        return ( mItemRef ) ? mItemRef->getParams() : emptyVec;
    }
    
    float getParamValue( std::string name ) { return ( mItemRef ) ? mItemRef->getParamValue( name ) : 0.0f; }

    bool isPlaying() { return ( mItemRef ) ? mItemRef->isPlaying() : false; }
    
    std::string getType() { return mType; }
    
protected:
    
    void registerParam( const std::string name, float initVal = 0.0f, float minVal = 0.0f, float maxVal = 1.0f )
    {
        mItemRef->registerParam( name, initVal, minVal, maxVal );
    }

    void registerParam( const std::string name, float *var, float minVal = 0.0f, float maxVal = 1.0f )
    {
        mItemRef->registerParam( name, var, minVal, maxVal );
    }

protected:
    
    std::string             mType;
    
    QTimelineItemRef        mItemRef;
    
    bool                    mMarkedForRemoval;
    
};

#endif

