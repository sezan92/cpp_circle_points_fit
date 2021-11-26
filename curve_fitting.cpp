#include <iostream>
#include "Eigen/Dense"
#include <fstream>
#include <cmath>
using namespace std;

// c++ implementation of this code https://meshlogic.github.io/posts/jupyter/curve-fitting/fitting-a-circle-to-cluster-of-3d-points/

void save_points(Eigen::MatrixXd &points, const string &name);
void get_projected_circle_points(double xc, double yc, double r, Eigen::VectorXd &xx, Eigen::VectorXd &yy, const int &num_points);

Eigen::MatrixXd get_projection(Eigen::Vector3d &k, Eigen::MatrixXd &points_centered, double &theta);
Eigen::VectorXd fit_circle_2d(Eigen::MatrixXd &points_proj);
Eigen::Vector3d fit_plane(Eigen::MatrixXd &points_centered);


int main(){
    Eigen::MatrixXd points(14, 3);
    points << 1.69394097, 0.89699047, 4.06276299,
       2.56823087, 0.69842007, 3.21497148,
       3.43271618, 0.98021214, 2.69155125,
       4.43213582, 1.85331114, 2.37948545,
       4.92328644, 2.82200303, 2.32861067,
       4.90166249, 3.95560872, 2.75117377,
       4.59295629, 4.80557214, 3.60124569,
       3.95215125, 5.45093418, 4.33841482,
       2.998557  , 5.257648  , 5.21266337,
       2.04399771, 4.61768856, 5.75436283,
       1.16719407, 3.63895906, 5.71525569,
       1.04464029, 2.60742983, 5.51914509,
       1.11656632, 1.59732638, 4.70627518,
       1.86951642, 0.90311449, 4.10275629;
    
    cout<<"\nINFO: Given Points:\n" << points << endl;
    save_points(points, "matrix.txt");
    Eigen::Vector3d origin = points.colwise().mean();
    cout << "\nINFO: Center of the given points:\n"<< origin <<endl;
    Eigen::MatrixXd points_centered = points.rowwise() - origin.transpose(); 
    cout << "\nINFO: Centered points, i.e. the making the origin (0, 0 , 0):\n" << points_centered << endl;
    Eigen::Vector3d normal = fit_plane(points_centered);
    cout <<"\nINFO: Normal vector of the fitting plane:\n" << normal;
    Eigen::Vector3d xy_normal;
    xy_normal << 0, 0, 1;
    Eigen::Vector3d k = (normal / normal.norm()).cross((xy_normal / xy_normal.norm())); // not sure if I need to divide by the norm
    double theta = acos(normal.dot(xy_normal));

    Eigen::MatrixXd points_proj = get_projection(k, points_centered, theta);
    cout<<"INFO: Projected points:\n"<<points_proj<<"\n"<<endl;
    save_points(points_proj, "projected_points.txt");
    
    Eigen::VectorXd c = fit_circle_2d(points_proj);
    double xc = c(0) / 2;
    double yc = c(1) / 2;
    double r = sqrt(c(2) + pow(xc, 2) + pow(yc, 2));
    const int num_points = 100;
    Eigen::VectorXd xx(num_points);
    Eigen::VectorXd yy(num_points);
    get_projected_circle_points(xc, yc, r, xx, yy, num_points);

    Eigen::MatrixXd proj_circle_points(num_points, 3);
    proj_circle_points.col(0) = xx.array();
    proj_circle_points.col(1) = yy.array();
    proj_circle_points.col(2) = Eigen::VectorXd::Ones(num_points)*0;

    save_points(proj_circle_points, "projected_circle_points_2d.txt");

    double theta_reverse = acos(xy_normal.dot(normal));
    Eigen::Vector3d k_reverse = (xy_normal / xy_normal.norm()).cross((normal / normal.norm()));
    Eigen::MatrixXd projected_circle_points_3d = get_projection(k_reverse, proj_circle_points, theta_reverse);

    save_points(projected_circle_points_3d, "projected_circle_points_3d.txt");

    Eigen::MatrixXd points_final = projected_circle_points_3d.rowwise() + origin.transpose();
    save_points(points_final, "final_points_3d.txt");
    return 0;
}

void save_points(Eigen::MatrixXd &points, const string &name){
    cout <<"INFO: Saving it to file:\n" << name << endl;
    std::ofstream file(name);
    if (file.is_open()){
        file << points << endl;
    }

}

Eigen::MatrixXd get_projection(Eigen::Vector3d &k, Eigen::MatrixXd &points_centered, double &theta){
    Eigen::MatrixXd points_proj(points_centered.rows(), points_centered.cols());
        points_proj = points_centered;

        for (int i=0; i<points_proj.rows(); i++){
            points_proj.row(i)(0) = k(1) * points_centered.row(i)(2) - k(2) * points_centered.row(i)(1);
            points_proj.row(i)(1) = k(2) * points_centered.row(i)(0) - k(0) * points_centered.row(i)(2);
            points_proj.row(i)(2) = k(0) * points_centered.row(i)(1) - k(1) * points_centered.row(i)(0);
            
            points_proj.row(i) = points_proj.row(i)*sin(theta);

            double dot_product = k.dot(points_centered.row(i));
            Eigen::Vector3d add_dot_product = k * dot_product * (1 - cos(theta));

            points_proj.row(i)(0) = points_proj.row(i)(0) + add_dot_product(0);
            points_proj.row(i)(1) = points_proj.row(i)(1) + add_dot_product(1);
            points_proj.row(i)(2) = points_proj.row(i)(2) + add_dot_product(2);
            
            points_proj.row(i) += points_centered.row(i) * cos(theta);
        }
    return points_proj;
}

Eigen::VectorXd fit_circle_2d(Eigen::MatrixXd &points_proj){

    Eigen::VectorXd x = points_proj.col(0).array();
    Eigen::VectorXd y = points_proj.col(1).array();
    Eigen::VectorXd b = x.cwiseProduct(x) + y.cwiseProduct(y);
    Eigen::MatrixXd A(points_proj.rows(), points_proj.cols());

    A.col(0) = x;
    A.col(1) = y;
    A.col(2) = Eigen::VectorXd::Ones(points_proj.rows());
    
    Eigen::VectorXd c = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    
    cout << "The least-squares solution is:\n"
        << c << endl;

    return c;

}

void get_projected_circle_points(double xc, double yc, double r, Eigen::VectorXd &xx, Eigen::VectorXd &yy, const int &num_points){

    double theta_increment = 2 * 3.14 / num_points;
    double t = 0;
    for(int i = 0; i< num_points; i++){
        xx(i) = xc + r * cos(t);
        yy(i) = yc + r * sin(t);
        t += theta_increment;

    }
}

Eigen::Vector3d fit_plane(Eigen::MatrixXd &points_centered){
        Eigen::BDCSVD<Eigen::MatrixXd> svd(points_centered, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Eigen::MatrixXd U = svd.matrixU();
        Eigen::MatrixXd V = svd.matrixV();
        cout<< "\nINFO: SVD U: \n"<< U << "\n SVD V:\n" << V << endl;
        
        // 3rd column is the normal of the fitting plane

        Eigen::Vector3d normal = V.col(2);

        return normal;

}