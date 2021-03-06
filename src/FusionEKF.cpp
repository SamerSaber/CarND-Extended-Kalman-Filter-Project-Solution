#include "FusionEKF.h"
#include "tools.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/*
 * Constructor.
 */
FusionEKF::FusionEKF() {
  is_initialized_ = false;

  previous_timestamp_ = 0;

  // initializing matrices
  R_laser_ = MatrixXd(2, 2);
  R_radar_ = MatrixXd(3, 3);
  H_laser_ = MatrixXd(2, 4);
  Hj_ = MatrixXd(3, 4);

  //measurement covariance matrix - laser
  R_laser_ << 0.0225, 0,
        0, 0.0225;

  //measurement covariance matrix - radar
  R_radar_ << 0.09, 0, 0,
        0, 0.0009, 0,
        0, 0, 0.09;

  /**
  TODO:
    * Finish initializing the FusionEKF.
    * Set the process and measurement noises
  */
  H_laser_ <<  1, 0, 0, 0,
		  	  	  	  	  0, 1, 0, 0;



}

/**
* Destructor.
*/
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {


  /*****************************************************************************
   *  Initialization
   ****************************************************************************/
  if (!is_initialized_) {
    /**
    TODO:
      * Initialize the state ekf_.x_ with the first measurement.
      * Create the covariance matrix.
      * Remember: you'll need to convert radar from polar to cartesian coordinates.
    */
    // first measurement
    cout << "EKF: " << endl;
    VectorXd X = VectorXd(4);
    MatrixXd P = MatrixXd(4,4);
    MatrixXd F= MatrixXd(4, 4);
    MatrixXd R;
    MatrixXd Q= MatrixXd(4, 4);


    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      /**
      Convert radar from polar to cartesian coordinates and initialize state.
      */
    	float x = measurement_pack.raw_measurements_(0) * cos(measurement_pack.raw_measurements_(1));
    	float y = measurement_pack.raw_measurements_(0) * sin(measurement_pack.raw_measurements_(1));
    	X << x, y, 0.0 , 0.0;
    	R = R_radar_;

    }
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      /**
      Initialize state.
      */
    	X <<  measurement_pack.raw_measurements_(0), measurement_pack.raw_measurements_(1), 0.0, 0.0;
    	R = R_laser_;
    }

    previous_timestamp_ = measurement_pack.timestamp_;

    P << 1, 0, 0, 0,
   	 0, 1, 0, 0,
	0, 0, 1000, 0,
	0, 0, 0, 1000;

       F << 1, 0, 0, 0,
    		   0, 1, 0, 0,
			   0, 0, 1, 0,
			   0, 0, 0, 1;

    ekf_.Init(X, P, F,H_laser_ , R, Q );
    // done initializing, no need to predict or update
    is_initialized_ = true;
    return;
  }

  /*****************************************************************************
   *  Prediction
   ****************************************************************************/

  /**
   TODO:
     * Update the state transition matrix F according to the new elapsed time.
      - Time is measured in seconds.
     * Update the process noise covariance matrix.
     * Use noise_ax = 9 and noise_ay = 9 for your Q matrix.
   */

  float dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;

      ekf_.F_ <<    1, 0, dt, 0,
							  0, 1, 0, dt,
							  0, 0, 1, 0,
							  0, 0, 0, 1;

      // Avoid too much computation while computing the Q matrix
      double dt_2 = pow(dt, 2);
      double dt_3 = pow(dt, 3) / 2;
      double dt_4 = pow(dt, 4) / 4;
      float noise_ax = 9;
      float noise_ay = 9;

      ekf_.Q_ <<  dt_4 * noise_ax, 0, dt_3 * noise_ax, 0,
                  0, dt_4 * noise_ay, 0, dt_3 * noise_ay,
                  dt_3 * noise_ax, 0, dt_2 * noise_ax, 0,
                  0, dt_3 * noise_ay, 0, dt_2 * noise_ay;


      ekf_.Predict();

  /*****************************************************************************
   *  Update
   ****************************************************************************/

  /**
   TODO:
     * Use the sensor type to perform the update step.
     * Update the state and covariance matrices.
   */

  if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
    // Radar updates
	  ekf_.R_ = R_radar_;
	  ekf_.H_ = tools.CalculateJacobian(ekf_.x_);
	  ekf_.UpdateEKF(measurement_pack.raw_measurements_);
  } else {
    // Laser updates
	  ekf_.R_ = R_laser_;
	  ekf_.H_ = H_laser_;
	  ekf_.Update(measurement_pack.raw_measurements_);
  }

  // print the output
  cout << "x_ = " << ekf_.x_ << endl;
  cout << "P_ = " << ekf_.P_ << endl;

  //update the previous time stamp
  previous_timestamp_ = measurement_pack.timestamp_;
}
