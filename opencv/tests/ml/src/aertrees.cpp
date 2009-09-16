/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "mltest.h"

class CV_ERTreesTest : public CV_TreesBaseTest
{
public:
    CV_ERTreesTest();
    ~CV_ERTreesTest();
protected:
    virtual int train( int test_case_idx );
    virtual float get_error() { return ertrees->calc_error( &data ); };
    CvERTrees *ertrees;
};

CV_ERTreesTest :: CV_ERTreesTest() : CV_TreesBaseTest("ertrees", "train and predict")
{
    ertrees = new CvERTrees();
}

CV_ERTreesTest :: ~CV_ERTreesTest()
{
    delete ertrees;
}

int CV_ERTreesTest :: train( int test_case_idx )
{
    int MAX_DEPTH, MIN_SAMPLE_COUNT, MAX_CATEGORIES, CV_FOLDS, NACTIVE_VARS, MAX_TREES_NUM;
    float REG_ACCURACY = 0, OOB_EPS = 0.0;
    bool USE_SURROGATE, IS_PRUNED;
    const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets_names, test_case_idx ))->data.str.ptr;     

    // read validation params
    CvFileNode* fnode = cvGetFileNodeByName( validation_fs, 0, "validation" ), *fnode1 = 0;
    fnode = cvGetFileNodeByName( validation_fs, fnode, name );
    fnode = cvGetFileNodeByName( validation_fs, fnode, data_name );
    fnode = cvGetFileNodeByName( validation_fs, fnode, "model_params" );
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "max_depth" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "MAX_DEPTH can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    MAX_DEPTH = fnode1->data.i;
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "min_sample_count" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "MIN_SAMPLE_COUNT can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    MIN_SAMPLE_COUNT = fnode1->data.i;
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "use_surrogate" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "USE_SURROGATE can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    USE_SURROGATE = (fnode1->data.i != 0);
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "max_categories" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "MAX_CATEGORIES can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    MAX_CATEGORIES = fnode1->data.i;
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "cv_folds" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "CV_FOLDS can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    CV_FOLDS = fnode1->data.i;
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "is_pruned" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "IS_PRUNED can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    IS_PRUNED = (fnode1->data.i != 0);
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "nactive_vars" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "NACTIVE_VARS can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    NACTIVE_VARS = fnode1->data.i;
    fnode1 = cvGetFileNodeByName( validation_fs, fnode, "max_trees_num" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "MAX_TREES_NUM can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    MAX_TREES_NUM = fnode1->data.i;

    if ( !ertrees->train( &data, CvRTParams( MAX_DEPTH, MIN_SAMPLE_COUNT, REG_ACCURACY,
            USE_SURROGATE, MAX_CATEGORIES, 0, false, // (calc_var_importance == true) <=> RF processes variable importance
            NACTIVE_VARS, MAX_TREES_NUM, OOB_EPS, CV_TERMCRIT_ITER)) )
    {
        ts->printf( CvTS::LOG, "in test case %d model training  was failed", test_case_idx );
        return CvTS::FAIL_INVALID_OUTPUT;
    }
    return CvTS::OK;
}

CV_ERTreesTest ertrees_test;

/* End of file. */
