package org.opencv.test;

import junit.framework.TestCase;

import org.opencv.CvType;
import org.opencv.Mat;
import org.opencv.Scalar;
import org.opencv.core;
import org.opencv.highgui;


public class OpenCVTestCase extends TestCase {
       
	protected static int matSize = 10;
    
	protected static Mat dst;
    
    //Naming notation: <channels info>_[depth]_[dimensions]_value
    //examples: gray0   - single channel 8U 2d Mat filled with 0
    //          grayRnd - single channel 8U 2d Mat filled with random numbers
    //          gray0_32f_1d - refactor ;)

	protected static Mat gray0;
	protected static Mat gray1;
	protected static Mat gray2;
	protected static Mat gray3;
	protected static Mat gray9;
	protected static Mat gray127;
	protected static Mat gray128;
	protected static Mat gray255;    
	protected static Mat grayRnd;
    
	protected static Mat gray_16u_256;    
    
	protected static Mat gray0_32f;
	protected static Mat gray1_32f;
	protected static Mat gray3_32f;
	protected static Mat gray9_32f;
	protected static Mat gray255_32f;        
	protected static Mat grayE_32f;
	protected static Mat grayRnd_32f;    
      
	protected static Mat gray0_32f_1d;
    
	protected static Mat gray0_64f;    
	protected static Mat gray0_64f_1d;
       
	protected static Mat rgba0;
	protected static Mat rgba128;
    
	protected static Mat rgbLena;
	protected static Mat grayChess;
	
	protected static Mat v1;
	protected static Mat v2;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        
        dst = new Mat();
        assertTrue(dst.empty());

        gray0 = new Mat(matSize, matSize, CvType.CV_8U); gray0.setTo(new Scalar(0.0));
        gray1 = new Mat(matSize, matSize, CvType.CV_8U); gray1.setTo(new Scalar(1.0));
        gray2 = new Mat(matSize, matSize, CvType.CV_8U); gray2.setTo(new Scalar(2.0));
        gray3 = new Mat(matSize, matSize, CvType.CV_8U); gray3.setTo(new Scalar(3.0));
        gray9 = new Mat(matSize, matSize, CvType.CV_8U); gray9.setTo(new Scalar(9.0));
        gray127 = new Mat(matSize, matSize, CvType.CV_8U); gray127.setTo(new Scalar(127.0));
        gray128 = new Mat(matSize, matSize, CvType.CV_8U); gray128.setTo(new Scalar(128.0));
        gray255 = new Mat(matSize, matSize, CvType.CV_8U); gray255.setTo(new Scalar(255.0));
        
        gray_16u_256 = new Mat(matSize, matSize, CvType.CV_16U); gray_16u_256.setTo(new Scalar(256));
        
        Mat low  = new Mat(1, 1, CvType.CV_16UC1, new Scalar(0));
        Mat high = new Mat(1, 1, CvType.CV_16UC1, new Scalar(256));
        grayRnd = new Mat(matSize, matSize, CvType.CV_8U); core.randu(grayRnd, low, high);
        
        gray0_32f = new Mat(matSize, matSize, CvType.CV_32F); gray0_32f.setTo(new Scalar(0.0));
        gray1_32f = new Mat(matSize, matSize, CvType.CV_32F); gray1_32f.setTo(new Scalar(1.0));
        gray3_32f = new Mat(matSize, matSize, CvType.CV_32F); gray3_32f.setTo(new Scalar(3.0));
        gray9_32f = new Mat(matSize, matSize, CvType.CV_32F); gray9_32f.setTo(new Scalar(9.0));
        gray255_32f = new Mat(matSize, matSize, CvType.CV_32F); gray255_32f.setTo(new Scalar(255.0));
        grayE_32f = new Mat(matSize, matSize, CvType.CV_32F); grayE_32f = Mat.eye(matSize, matSize, CvType.CV_32FC1);
        grayRnd_32f = new Mat(matSize, matSize, CvType.CV_32F); core.randu(grayRnd_32f, low, high);
        
        gray0_32f_1d = new Mat(1, matSize, CvType.CV_32F); gray0_32f_1d.setTo(new Scalar(0.0));
        
        gray0_64f = new Mat(matSize, matSize, CvType.CV_64F); gray0_64f.setTo(new Scalar(0.0));
        gray0_64f_1d = new Mat(1, matSize, CvType.CV_64F); gray0_64f_1d.setTo(new Scalar(0.0));

        rgba0 = new Mat(matSize, matSize, CvType.CV_8UC4); rgba0.setTo(Scalar.all(0));
        rgba128 = new Mat(matSize, matSize, CvType.CV_8UC4); rgba128.setTo(Scalar.all(128));
        
        rgbLena = highgui.imread(OpenCVTestRunner.LENA_PATH);
        grayChess = highgui.imread(OpenCVTestRunner.CHESS_PATH);
        
		v1 = new Mat(1, 3, CvType.CV_32F); v1.put(0, 0, 1.0, 3.0, 2.0);
		v2 = new Mat(1, 3, CvType.CV_32F); v2.put(0, 0, 2.0, 1.0, 3.0);
    }    

    public static void assertMatEqual(Mat m1, Mat m2) {
    	//OpenCVTestRunner.Log(m1.toString());
    	//OpenCVTestRunner.Log(m2.toString());
    	
    	if (!m1.type().equals(m2.type()) || 
    	    m1.cols() != m2.cols() || m1.rows() != m2.rows()) {
    		throw new UnsupportedOperationException();
    	}
    	else if (m1.channels() == 1) {
    		assertTrue(CalcPercentageOfDifference(m1, m2) == 0.0);
    	}
    	else {
    		for (int coi = 0; coi < m1.channels(); coi++) {
    			Mat m1c = getCOI(m1, coi);
    			Mat m2c = getCOI(m2, coi);
    			assertTrue(CalcPercentageOfDifference(m1c, m2c) == 0.0);
    		}
    	}
    }
    
    static private Mat getCOI(Mat m, int coi) {
    	Mat ch = new Mat(m.rows(), m.cols(), m.depth());
    	
    	for (int i = 0; i < m.rows(); i++)
    		for (int j = 0; j < m.cols(); j++)
    		{
    			double pixel[] = m.get(i, j);
    			ch.put(i, j, pixel[coi]);
    		}    			
    	
    	return ch;
    }

    static private double CalcPercentageOfDifference(Mat m1, Mat m2) {
        Mat cmp = new Mat(0, 0, CvType.CV_8U);
        core.compare(m1, m2, cmp, core.CMP_EQ);
        double num = 100.0 * 
            (1.0 - Double.valueOf(core.countNonZero(cmp)) / Double.valueOf(cmp.rows() * cmp.cols()));

        return num;
    }

    public void test_1(String label) {
    	OpenCVTestRunner.Log("================================================");
    	OpenCVTestRunner.Log("=============== " + label);
    }
}
