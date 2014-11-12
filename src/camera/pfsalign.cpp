/**
 * @file pfsalign.cpp
 * @brief Image stack alignment using feature matching
 *
 * The code relies on OpenCV SURF feature detector / descriptor. The ideas for
 * prunning false matches are mostly based on the book:
 *
 * Lagani, R. (2011). OpenCV 2 Computer Vision Application Programming Cookbook.
 * Packt Publishing.
 *
 *
 * This file is a part of pfscalibration package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2003-2013 Rafal Mantiuk and Grzegorz Krawczyk
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ----------------------------------------------------------------------
 *
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: pfsalign.cpp,v 1.11 2013/11/14 21:28:28 rafm Exp $
 */

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <getopt.h>
#include <string.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <libexif/exif-data.h>

#include <pfs.h>

#define PROG_NAME "pfsalign"
#define VERBOSE_STR if( verbose ) std::cerr << PROG_NAME ": "

bool verbose = false;

class QuietException
{
};

using namespace cv;
using namespace std;

void printHelp()
{
    fprintf( stderr, PROG_NAME " [-r index|--reference index] [-c (min|max)|--crop (min|max)] [-d|--display-matches] [-f|--fail-no-match] -v -h\n"
             "See man page for more information.\n" );
}

class Toc
{
    double tickCount;
public:
    void tic( const char *name = NULL )
    {
        if( name != NULL ) {
            VERBOSE_STR << "Starting " << name << "...";
        } else {
            VERBOSE_STR << "Starting processing ...";
        }
        tickCount = static_cast<double>( getTickCount() );
    }

    void toc( )
    {
        double duration = (static_cast<double>( getTickCount() ) - tickCount) / getTickFrequency();
        if( verbose )
            std::cerr << "completed. It took " << duration << " seconds." << std::endl;
    }
};



void pruneNNDR( std::vector<std::vector<DMatch> > &matches, float ratio )
{
    for( vector<vector<DMatch> >::iterator it = matches.begin(); it != matches.end(); it++ ) {
        bool prune = false;
        if( it->size() < 2 )
            prune = true;
        else {
            float NNDR = (*it)[0].distance / (*it)[1].distance;
            if( NNDR >= ratio )
                prune = true;
        }
        if( prune )
            it->clear();
    }

}

void symmetryTest( const vector<vector<DMatch> > &matches_12, const vector<vector<DMatch> > &matches_21, vector<DMatch> &matches_sym )
{
    for( vector< vector<DMatch> >::const_iterator it1 = matches_12.begin(); it1 != matches_12.end(); it1++ ) {
        if( it1->size() == 0 )
            continue;

        for( vector< vector<DMatch> >::const_iterator it2 = matches_21.begin(); it2 != matches_21.end(); it2++ ) {
            if( it2->size() == 0 )
                continue;

            if( (*it1)[0].queryIdx == (*it2)[0].trainIdx && (*it2)[0].trainIdx == (*it1)[0].queryIdx ) {
                matches_sym.push_back( DMatch( (*it1)[0].queryIdx, (*it1)[0].trainIdx, (*it1)[0].distance ));
                break;
            }
        }

    }
}


/**
 Match feature points in a pair image and find homography.
  */
bool alignImagePair( const Mat &ref_img, const Mat &exp_img, Mat &homography, int sensitivity, bool display_matches )
{
    Toc toc;

    homography = Mat::eye( 3, 3, CV_64F );
    //    cv::imshow( "Result", ref_img );
    //    cv::imshow( "Result2", exp_img );
    //    cv::waitKey(0);

    Ptr<FeatureDetector> detector(new DynamicAdaptedFeatureDetector( new SurfAdjuster( (11-sensitivity) * 100.f, 2, 1000 ),
                                                                     100, 1000, sensitivity/2+2 ));
//    Ptr<FeatureDetector> detector;
    //    detector = new GoodFeaturesToTrackDetector();
//    detector = new SurfFeatureDetector();

    std::vector<KeyPoint> keypoints1, keypoints2;

    toc.tic( "feature detection" );
    detector->detect( ref_img, keypoints1 );
    detector->detect( exp_img, keypoints2 );
    toc.toc( );

    if( keypoints1.size() < 10 || keypoints2.size() < 10 ) {
        cerr << PROG_NAME ": Could not detect a sufficient number of keypoints (found "
             << keypoints1.size()  << " and " << keypoints2.size() << " keypoints)" << endl;

        if( display_matches ) {
            Mat vis;
            vector<char> inliners_c;
            std::vector<cv::DMatch> matches_sym;
            drawMatches( ref_img, keypoints1, exp_img, keypoints2, matches_sym, vis, Scalar(0,0,255), Scalar(0,255,255), inliners_c, DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
            cv::namedWindow( "Image pair matches" );
            cv::imshow( "Image pair matches", vis );
            cv::waitKey(0);
        }

        return false;
    }

    //        SiftDescriptorExtractor surfDesc;
    SurfDescriptorExtractor surfDesc;

    Mat descr_ref, descr_exp;
    toc.tic( "descriptor extraction" );
    surfDesc.compute( ref_img, keypoints1, descr_ref );
    surfDesc.compute( exp_img, keypoints2, descr_exp );
    toc.toc( );

    FlannBasedMatcher matcher;
    //        BruteForceMatcher< cv::L2<float> > matcher;

    toc.tic( "matching" );
    std::vector< std::vector<cv::DMatch> > matches_rt;
    matcher.knnMatch( descr_ref, descr_exp, matches_rt, 2 );

    std::vector< std::vector<cv::DMatch> > matches_tr;
    matcher.knnMatch( descr_exp, descr_ref, matches_tr, 2 );

    pruneNNDR( matches_rt, 1 );
    pruneNNDR( matches_tr, 1 );

    std::vector<cv::DMatch> matches_sym;
    symmetryTest( matches_rt, matches_tr, matches_sym );
    toc.toc( );

    std::vector<cv::DMatch>::iterator it;
    vector<Point3f> p1, p2;
    Mat_<float> pp1(matches_sym.size(),2);
    Mat_<float> pp2(matches_sym.size(),2);
    int kk = 0;
    for( it = matches_sym.begin(); it != matches_sym.end(); it++, kk++ ) {

        const Point2f from = keypoints1[it->queryIdx].pt;
        p1.push_back( Point3f( from.x, from.y, 1 ) );
        pp1(kk,0) = from.x;
        pp1(kk,1) = from.y;
        //        pp1(kk,2) = 1;
        const Point2f to = keypoints2[it->trainIdx].pt;
        p2.push_back( Point3f( to.x, to.y, 1 ) );
        pp2(kk,0) = to.x;
        pp2(kk,1) = to.y;
        //pp2(kk,2) = 1;
        //            std::cout << "Matches: " << from << " -> " << to << std::endl;
    }

    if( matches_sym.size() < 9 ) {
        cerr << PROG_NAME ": Not enough matches found between a pair of images (found " << matches_sym.size() << ")" << endl;
        return false;
    }

    //    Mat affine = estimateRigidTransform( e1, e2, false );

    vector<uchar> inliners(matches_sym.size(), 0);
    //        affine = findHomography( pp1, pp2, inliners, CV_RANSAC, 1. );
    homography = findHomography( pp2, pp1, CV_RANSAC, 1., inliners );

    //    solve( pp1, pp2, affine, DECOMP_SVD );
    //    Mat affine = (Mat_<float>(2,3) << 1, 0, 0, 0, 1, 10);
    int inliner_count = count( inliners.begin(), inliners.end(), 1 );

    VERBOSE_STR << "Found " << matches_sym.size() << " matching features, reduced to " << inliner_count << " inliners." << endl;

    if( display_matches ) {
        Mat vis;
        vector<char> inliners_c(matches_sym.size(), 0);
        for( size_t i = 0; i < inliners.size(); i++ )
            inliners_c[i] = (char)inliners[i];

        drawMatches( ref_img, keypoints1, exp_img, keypoints2, matches_sym, vis, Scalar(0,0,255), Scalar(0,255,255), inliners_c, DrawMatchesFlags::DRAW_RICH_KEYPOINTS|DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

        cv::namedWindow( "Image pair matches" );
        cv::imshow( "Image pair matches", vis );

        cv::waitKey(0);
    }

    if( inliner_count < 9 ) {
        homography = Mat::eye( 3, 3, CV_64F );
        cerr << PROG_NAME ": Not enough inliners to find a reliable transformation." << endl;
        return false;
    }


    return true;

}


/**
  Convert floating point image to 8-bit image.
  */
Mat convert_to_8bit( Mat &in, bool apply_gamma )
{
    Mat out;
    if( !apply_gamma ) {
        in.convertTo( out, CV_8UC3, 255 );
    } else {
        out.create( in.rows, in.cols, CV_8UC3 );
        for(int i = 0; i < in.rows; i++)
        {
            Vec3f *i_p = in.ptr<Vec3f>(i);
            Vec3b *o_p = out.ptr<Vec3b>(i);
            for(int j = 0; j < in.cols; j++) {
                for( int cc=0; cc < 3; cc++ ) {
                    o_p[j][cc] = saturate_cast<uchar>(powf( i_p[j][cc], 1.f/2.2f ) * 255.f);
                }
            }
        }


    }


    return out;
}


// Find maximum and positive minimum for an image
void find_min_max( Mat &in, float &minV, float &maxV )
{
    minV = 1e10f;
    maxV = -1e10f;
    for(int i = 0; i < in.rows; i++)
    {
        Vec3f *i_p = in.ptr<Vec3f>(i);
        for(int j = 0; j < in.cols; j++) {
            for( int cc=0; cc < 3; cc++ ) {
                float v = i_p[j][cc];
                if( v > 0 ) {
                    if( v < minV )
                        minV = v;
                    if( v > maxV )
                        maxV = v;
                }
            }
        }
    }

}

Mat convert_and_log_scale( Mat &in, float min, float max, float mult, float gamma )
{
    Mat out;
    out.create( in.rows, in.cols, CV_8UC3 );

    const float l_min = logf( min );
    const float l_max = logf( max );
    const float l_mult = logf( mult );

    for(int i = 0; i < in.rows; i++)
    {
        Vec3f *i_p = in.ptr<Vec3f>(i);
        Vec3b *o_p = out.ptr<Vec3b>(i);
        for(int j = 0; j < in.cols; j++) {
            for( int cc=0; cc < 3; cc++ ) {
                if( i_p[j][cc] > 0.f )
                    o_p[j][cc] = saturate_cast<uchar>( ((gamma*(logf(i_p[j][cc]) - l_max) + l_mult) / (l_max-l_min) + 1.f)  * 255.f);
                else
                    o_p[j][cc] = 0;
            }
        }
    }

    return out;
}

/**
  Convert a pair of floating point images to 8-bit.
  */
void convert_to_8bit( Mat &in1, Mat &in2, Mat &out1, Mat &out2, float exp_mult1, float exp_mult2, bool is_ldr )
{
    if( is_ldr && exp_mult1 == 1.f && exp_mult2 == 1.f ) {
        in1.convertTo( out1, CV_8UC3, 255*exp_mult1);
        in2.convertTo( out2, CV_8UC3, 255*exp_mult2);
    } else {
        float minV1, minV2, maxV1, maxV2;
        find_min_max( in1, minV1, maxV1 );
        find_min_max( in2, minV2, maxV2 );

        minV1 *= exp_mult1;
        maxV1 *= exp_mult1;
        minV2 *= exp_mult2;
        maxV2 *= exp_mult2;

        float gamma = is_ldr ? 2.2f : 1.f;

        minV1 = powf( minV1, gamma );
        minV2 = powf( minV2, gamma );

        float minV, maxV;
        minV = std::min( minV1, minV2 );
        maxV = std::max( maxV1, maxV2 );


        if( (maxV/minV) > 10000.f ) {
            // To avoid compressing contrast too much
            minV = maxV / 10000.f;
        }
        if( (maxV/minV) < 100.f ) {
            // To avoid streching contrast too much
            minV = maxV / 100.f;
        }

        VERBOSE_STR << "Using log scaling (min val: " << minV1 << " max value: " << maxV1 << ")" << endl;


        out1 = convert_and_log_scale( in1, minV, maxV1, exp_mult1, gamma );
        out2 = convert_and_log_scale( in2, minV, maxV1, exp_mult2, gamma );
    }
}

Point2f intersect_lines( Point2f p1, Point2f p2, Point2f p3, Point2f p4 )
{
    float s = ((p4.x-p3.x)*(p3.y-p1.y) - (p3.x-p1.x)*(p4.y-p3.y)) / ( (p4.x-p3.x)*(p2.y-p1.y) - (p2.x-p1.x)*(p4.y-p3.y) );

    Point2f p;
    p.x = p1.x + s * (p2.x-p1.x);
    p.y = p1.y + s * (p2.y-p1.y);

    return p;
}

Rect auto_crop( const Size2f &size, const vector<Point2f> &rotated )
{

    assert( rotated.size() == 4 ); // 4 corners of the polygon

    Point2f centre;
    for( int kk=0; kk < 4; kk++ )
        centre += rotated[kk];

    centre = centre * 0.25;

    //cout << "Float: " << (rot_mat.type() == CV_32F) << endl;

    float best_d = 1e10;
    Point2d best_p;
    for( int jj = 0; jj < 2; jj++ ) {
        for( int ii=0; ii < 2; ii++ ) {
            Point2f to;
            if( jj == 0 )
                to = centre - Point2f( size.width/2, size.height/2 );
            else
                to = centre - Point2f( -size.width/2, size.height/2 );

            Point2f p1 = intersect_lines( centre, to, rotated[ii], rotated[ii+1] );
            float d = powf(p1.x-centre.x,2)+powf(p1.y-centre.y,2);
            if( d < best_d ) {
                best_d = d;
                best_p = p1;
            }
        }
    }
    Point2f ul( centre.x - fabs(best_p.x-centre.x), centre.y - fabs(best_p.y-centre.y) );
    Point2f br( centre.x + fabs(best_p.x-centre.x), centre.y + fabs(best_p.y-centre.y) );

    Rect crop( ul, br );

    return crop;
}

struct FrameInfo {
    Mat image;
    std::string file_name;
    pfs::Frame *frame;
    Size size;

    FrameInfo( Mat &image, const char *file_name, pfs::Frame *frame ) :
        image( image ), file_name( file_name ), frame( frame )
    {
        size = Size( image.cols, image.rows );
    }
};

void alignFrames(int argc, char *argv[])
{
    pfs::DOMIO pfsio;

    int reference_index = 0; // Index of the reference image
    bool display_matches = false;
    bool crop_min = false;
    bool fail_on_no_match = false;
    int sensitivity = 5;

    static struct option cmdLineOptions[] = {
        { "help", no_argument, NULL, 'h' },
        { "verbose", no_argument, NULL, 'v' },
        { "reference", required_argument, NULL, 'r' },
        { "crop", required_argument, NULL, 'c' },
        { "sensitivity", required_argument, NULL, 's' },
        { "fail-no-match", no_argument, NULL, 'f' },
        { "display-matches", no_argument, NULL, 'd' },
        { NULL, 0, NULL, 0 }
    };

    int optionIndex = 0;
    while( 1 ) {
        int c = getopt_long (argc, argv, "r:c:s:fhvd", cmdLineOptions, &optionIndex);
        if( c == -1 ) break;
        switch( c ) {
        case 'h':
            printHelp();
            throw QuietException();
        case 'v':
            verbose = true;
            break;
        case 'r':
            reference_index = strtol( optarg, NULL, 10 ) - 1;
            break;
        case 'd':
            display_matches = true;
            break;
        case 'c':
            if( !strcasecmp( optarg, "min" ) ) {
                crop_min = true;
            } else if( !strcasecmp( optarg, "max" ) ) {
                crop_min = false;
            } else
                throw pfs::Exception( "Unrecognized cropping mode" );
            break;
        case 'f':
            fail_on_no_match = true;
            break;
        case 's':
            sensitivity = strtol( optarg, NULL, 10 );
            if( sensitivity <= 0 || sensitivity > 10 ) {
                throw pfs::Exception( "Sensitivity parameter must be within 1-10 range.");
            }
            break;
        case '?':
            throw QuietException();
        case ':':
            throw QuietException();
        }
    }

    vector<FrameInfo> frames;

    if( crop_min ) {
        VERBOSE_STR << "Cropping to the size that contains only overlaping pixels (min)" << endl;
    } else {
        VERBOSE_STR << "Cropping to the size that contains all pixels (max)" << endl;
    }


    // Load all images
    //        bool first_frame = true;
    int frame_count = 0;
    for( ; true; frame_count++ ) {

        pfs::Frame *frame = pfsio.readFrame( stdin );
        if( frame == NULL ) break; // No more frames

        const char *f_name = frame->getTags()->getString("FILE_NAME");
        if( f_name == NULL ) {
            std::stringstream f_name_str;
            f_name_str << "frame_" << frame_count;
            f_name = f_name_str.str().c_str();
        }
        VERBOSE_STR << "Loading " << f_name << endl;

        pfs::Channel *X, *Y, *Z;
        frame->getXYZChannels( X, Y, Z );

        if( X != NULL ) {           // Color, XYZ
            pfs::Channel* &R = X;
            pfs::Channel* &G = Y;
            pfs::Channel* &B = Z;
            pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, R, G, B );

            Mat img;
            img.create( frame->getHeight(), frame->getWidth(), CV_32FC3 );

            int in_index = 0;
            for(int i = 0; i < img.rows; i++)
            {
                Vec3f *o_p = img.ptr<Vec3f>(i);
                for(int j = 0; j < img.cols; j++) {
                    o_p[j][0] = (*B)(in_index);
                    o_p[j][1] = (*G)(in_index);
                    o_p[j][2] = (*R)(in_index);
                    in_index++;
                }
            }

            frames.push_back( FrameInfo( img, f_name, frame ) );

        } else
            throw pfs::Exception( "Only color images are supported" );


        // Remove all channels from the frame to save memory
        pfs::ChannelIteratorPtr cit( frame->getChannelIterator() );
        while( cit->hasNext() ) {
            pfs::Channel *ch = cit->getNext();
            frame->removeChannel( ch );
        }

    }

    if( reference_index < 0 || (size_t)reference_index >= frames.size() )
        throw pfs::Exception( "Reference image index out of range" );


    vector<Mat> homoM;
    vector<Size> imageSizes;
    for( int ff=1; ff < (int)frames.size(); ff++ ) {
        VERBOSE_STR << "Processing " <<  frames[ff-1].file_name << " and " << frames[ff].file_name << endl;

        cv::Mat ref_img = frames[ff-1].image;
        cv::Mat exp_img = frames[ff].image;

        imageSizes.push_back( exp_img.size() );

        bool is_ldr = false;
        const char *lum_type = frames[ff].frame->getTags()->getString("LUMINANCE");
        if( lum_type ) {
            if( !strcmp( lum_type, "DISPLAY" ) )
                is_ldr = true;
        }

        // Extract exposure values
        float exp1 = 1.f, exp2 = 1.f;
        const char* exp_str = frames[ff-1].frame->getTags()->getString("BV");
        if( exp_str != NULL )
            exp1 =  powf(2.0f,strtof( exp_str, NULL ));
        exp_str = frames[ff].frame->getTags()->getString("BV");
        if( exp_str != NULL )
            exp2 = powf(2.0f,strtof( exp_str, NULL ));

        if( exp1 / exp2 != 1.f ) {
            VERBOSE_STR << "Exposure pair: " << exp1 << ", " << exp2 << " (" << exp2/exp1 << " ratio)" << endl;
        }

        const float exp_mult1 = exp1 / std::min( exp1, exp2 );
        const float exp_mult2 = exp2 / std::min( exp1, exp2 );

        Mat ref_img_8b, exp_img_8b;

        //        Mat ref_img_8b = convert_to_8bit( ref_img, apply_gamma );
        //        Mat exp_img_8b = convert_to_8bit( exp_img, apply_gamma );
        //        convert_to_8bit( ref_img, exp_img, ref_img_8b, exp_img_8b, 1, 1, is_ldr );
        convert_to_8bit( ref_img, exp_img, ref_img_8b, exp_img_8b, exp_mult1, exp_mult2, is_ldr );

        Mat homography;
        bool success = alignImagePair( ref_img_8b, exp_img_8b, homography, sensitivity, display_matches );
        if( !success && fail_on_no_match )
            throw pfs::Exception( "Stopping because could not find a match between image pair");

        VERBOSE_STR << "Homography (for the image pair):" << endl << homography << endl;

        homoM.push_back( homography );
    }


    // Chain all transformations and find the cropping box
    vector<Mat> homoMC;
    Rect_<double> pano_rect( 0, 0, frames[0].image.cols, frames[0].image.rows );
    for( int kk = 0; kk < (int)frames.size(); kk++ )
    {
        Mat_<double> trans = Mat::eye(3,3,CV_64F);
        for( int ll = min( kk, reference_index )+1; ll <= max( kk, reference_index); ll++ )
        {
            if( ll > 0 )
                trans = trans*homoM[ll-1];
        }
        if( kk < reference_index )
            trans = trans.inv();

        homoMC.push_back( trans );

        double data[4][3] = { { 0, 0, 1 }, { frames[kk].size.width, 0, 1 }, { frames[kk].size.width, frames[kk].size.height, 1 }, { 0, frames[kk].size.height, 1 } };
        Mat corners( 4, 3, CV_64F, data );

        Mat corners_trans = trans * corners.t();

        //      VERBOSE_STR << "Image: " << fileNames[kk] << endl;
        //      VERBOSE_STR << " Corners: " << trans * corners.t() << endl;

        Mat p_nh; //( 4, 3, CV_32F );
        Mat_<float> corners_f;
        corners_trans.convertTo( corners_f, CV_32F );
        convertPointsFromHomogeneous( corners_f.t(), p_nh );


        if( crop_min ) {
            vector<Point2f> dest_rect(4);
            for( int ii=0; ii < 4; ii++ ) {
                dest_rect[ii].x = p_nh.at<float>(ii,0);
                dest_rect[ii].y = p_nh.at<float>(ii,1);
            }
            Rect_<float> img_rect = auto_crop( frames[kk].size, dest_rect );
            Rect_<double> img_rect_d = img_rect;
            pano_rect = pano_rect & img_rect_d;
        } else {
            Rect_<double> img_rect = boundingRect( p_nh );
            pano_rect = pano_rect | img_rect;
        }


        //        VERBOSE_STR << "Bounding rect: " << pano_rect.x << ", " << pano_rect.y << " - " << pano_rect.width << "x" << pano_rect.height << endl;
    }


    // Align
    Size dest_size = Size( ceil( pano_rect.width ), ceil( pano_rect.height) );
    //    Mat pano_img( ceil( pano_rect.height), ceil( pano_rect.width ), CV_8UC3 );
    //    pano_img.setTo( Vec3b(255, 255, 255 ));
    for( size_t ff=0; ff < frames.size(); ff++ ) {
        VERBOSE_STR << "Warping: "<<  frames[ff].file_name << endl;

        Mat exp_img = frames[ff].image;

        Mat_<double> translate = Mat::eye(3,3,CV_64F);
        translate(0,2) = -pano_rect.x;
        translate(1,2) = -pano_rect.y;
        Mat trans = translate*homoMC[ff];
        VERBOSE_STR << "Homography (to reference): " << trans << endl;

        Mat res_img;
        warpPerspective( exp_img, res_img, trans, dest_size, INTER_LINEAR );

        pfs::Frame *alignedFrame = NULL;
        alignedFrame = pfsio.createFrame( res_img.cols, res_img.rows );

        pfs::Channel *X = alignedFrame->createChannel( "X" );
        pfs::Channel *Y = alignedFrame->createChannel( "Y" );
        pfs::Channel *Z = alignedFrame->createChannel( "Z" );

        pfs::Channel* &R = X;
        pfs::Channel* &G = Y;
        pfs::Channel* &B = Z;

        int out_index = 0;
        for(int i = 0; i < res_img.rows; i++)
        {
            Vec3f *i_p = res_img.ptr<Vec3f>(i);
            for(int j = 0; j < res_img.cols; j++) {
                (*B)(out_index) = i_p[j][0];
                (*G)(out_index) = i_p[j][1];
                (*R)(out_index) = i_p[j][2];
                out_index++;
            }
        }
        pfs::transformColorSpace( pfs::CS_RGB, R, G, B, pfs::CS_XYZ, X, Y, Z );
        pfs::copyTags( frames[ff].frame, alignedFrame );

        pfsio.writeFrame( alignedFrame, stdout );

        pfsio.freeFrame( alignedFrame );
        pfsio.freeFrame( frames[ff].frame );


        // TODO: Add alpha channel
        /*        Mat in_mask( exp_img.size(), CV_8U ), out_mask;
        in_mask.setTo( Scalar( 255 ) );
        warpPerspective( in_mask, out_mask, trans, pano_img.size(), INTER_LINEAR );
        res_img.copyTo( pano_img, out_mask );

        double data[4][3] = { { 0, 0, 1 }, { exp_img.cols, 0, 1 }, { exp_img.cols, exp_img.rows, 1 }, { 0, exp_img.rows, 1 } };
        Mat corners( 4, 3, CV_64F, data );
        Mat_<double> corners_trans = trans * corners.t();
        Mat p_nh; //( 4, 3, CV_32F );
        Mat_<float> corners_f;
        corners_trans.convertTo( corners_f, CV_32F );
        convertPointsFromHomogeneous( corners_f.t(), p_nh );*/
        //        Scalar borderColor = CV_RGB( 0, 0, 0 );
        //        line( pano_img, Point2f( p_nh(0,0), p_nh(0,1) ), Point2f( p_nh(1,0), p_nh(1,1) ), borderColor, 3 );
        //       line( pano_img, Point2f( p_nh(1,0), p_nh(1,1) ), Point2f( p_nh(2,0), p_nh(2,1) ), borderColor, 3 );
        //       line( pano_img, Point2f( p_nh(2,0), p_nh(2,1) ), Point2f( p_nh(3,0), p_nh(3,1) ), borderColor, 3 );
        //       line( pano_img, Point2f( p_nh(3,0), p_nh(3,1) ), Point2f( p_nh(0,0), p_nh(0,1) ), borderColor, 3 );
    }

}

int main( int argc, char* argv[] )
{
    try {
        alignFrames( argc, argv );
    }
    catch( pfs::Exception ex ) {
        fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
        return EXIT_FAILURE;
    }
    catch( QuietException  ex ) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
