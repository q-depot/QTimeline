/*
 *  QTimelineModule.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_MODULE
#define QTIMELINE_MODULE

#pragma once

#include "QTimelineModuleItem.h"

typedef std::shared_ptr<class QTimelineModule>    QTimelineModuleRef;


class QTimelineModule
{
    
public:
    
    QTimelineModule( std::string name, std::string type ) : mName(name), mType(type), mMarkedForRemoval(false) {}
    
    virtual ~QTimelineModule()
    {
        ci::app::console() << "delete QTimelineModule base class: " << std::endl;
        mModuleItemRef.reset();
    }
    
    virtual void update() {}
    
    virtual void render() {}
    
    virtual void init() {}
    
    std::string getName() { return mName; }
    
    void setItemRef( QTimelineModuleItemRef ref ) { mModuleItemRef = ref; }
    
    QTimelineModuleItemRef getItemRef() { return mModuleItemRef; }
    
    int getParamsN() { return ( mModuleItemRef ) ? mModuleItemRef->mParams.size() : 0; }
    
    std::vector<QTimelineParamRef> getParams()
    {
        std::vector<QTimelineParamRef> emptyVec;
        return ( mModuleItemRef ) ? mModuleItemRef->mParams : emptyVec;
    }
    
    float getParamValue( std::string name ) { return ( mModuleItemRef ) ? mModuleItemRef->getParamValue( name ) : 0.0f; }

    bool isPlaying() { return ( mModuleItemRef ) ? mModuleItemRef->isPlaying() : false; }
    
    std::string getType() { return mType; }
    
//    void markedForRemoval() { mMarkedForRemoval = true; }
    
protected:
    
    void registerParam( const std::string name, float initVal = 0.0f, float minVal = 0.0f, float maxVal = 1.0f )
    {
        mModuleItemRef->registerParam( name, initVal, minVal, maxVal );
    }

    void registerParam( const std::string name, float *var, float minVal = 0.0f, float maxVal = 1.0f )
    {
        mModuleItemRef->registerParam( name, var, minVal, maxVal );
    }

protected:
    
    std::string             mName;
    std::string             mType;
    
    QTimelineModuleItemRef  mModuleItemRef;
    
    bool                    mMarkedForRemoval;
};

#endif