#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QScreen>
#include <QScrollBar>
#include <QGridLayout>
#include <QtMath>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QPointLight>

#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>

//#include <QtCore/QDebug>
#define INDENT  1.0f
#define RADIUS 200.0f

float sin(float angle)
{
  return qSin(qDegreesToRadians(angle));
}
float cos(float angle)
{
  return qCos(qDegreesToRadians(angle));
}
// l - некая единица измерения
// радиус атома = 1l
// расстояние между центрами атомов 4l
// радиус цилинда трубки связи 0,25l
const float l     = 2.0f;
//-------------------------------------------------------------------------
void setCameraPos(Qt3DRender::QCamera* camera, const float x, const float y, const float z)
{
  QVector3D vector;
  vector.setX(x);
  vector.setY(y * (-1));
  vector.setZ(qSqrt(z*z - x*x - y*y));
  camera->setPosition(vector);
  camera->setUpVector(QVector3D(0.0f, qSqrt(z*z - y*y)/z, y/z));
  camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
}
//-------------------------------------------------------------------------
class Cylinder: public Qt3DCore::QEntity
{
public:
  Cylinder(Qt3DCore::QEntity* root, const float x, const float y, const float z, const float angle): Qt3DCore::QEntity(root)
  {
    Qt3DExtras::QCylinderMesh* cylinderMesh = new Qt3DExtras::QCylinderMesh();
    cylinderMesh->setRadius(0.25f * l);
    cylinderMesh->setLength(4.0f * l);
    cylinderMesh->setRings(2);
    cylinderMesh->setSlices(20);
    // CylinderMesh Transform
    Qt3DCore::QTransform *cylinderTransform = new Qt3DCore::QTransform();
    //cylinderTransform->setScale(1.0f);
    cylinderTransform->setRotationZ(angle);
    cylinderTransform->setTranslation(QVector3D(x - 2.0*l*qSin(qDegreesToRadians(angle)), y + 2.0*l*qCos(qDegreesToRadians(angle)), z));
    Qt3DExtras::QPhongMaterial* cylinderMaterial = new Qt3DExtras::QPhongMaterial();
    cylinderMaterial->setDiffuse(QColor(QRgb(0x000000)));
    // Cylinder
    addComponent(cylinderMesh);
    addComponent(cylinderMaterial);
    addComponent(cylinderTransform);
  }
};
//-------------------------------------------------------------------------
class Sphere: public Qt3DCore::QEntity
{
public:
  Sphere(Qt3DCore::QEntity* root, const float x, const float y, const float z): Qt3DCore::QEntity(root)
  {
    Qt3DExtras::QSphereMesh *sphereMesh = new Qt3DExtras::QSphereMesh();
    sphereMesh->setRings(20);
    sphereMesh->setSlices(20);
    sphereMesh->setRadius(l);
    // Sphere mesh transform
    Qt3DCore::QTransform *sphereTransform = new Qt3DCore::QTransform();
    sphereTransform->setScale(1.0f);
    sphereTransform->setTranslation(QVector3D(x, y, z));
    Qt3DExtras::QPhongMaterial *sphereMaterial = new Qt3DExtras::QPhongMaterial();
    sphereMaterial->setDiffuse(QColor(QRgb(0x7fa3db)));
    // Sphere
    addComponent(sphereMesh);
    addComponent(sphereMaterial);
    addComponent(sphereTransform);
  }
};
//-------------------------------------------------------------------------
class MiddleElement: public Qt3DCore::QEntity
{
public:
  MiddleElement(Qt3DCore::QEntity* root, const float x, const float y, const float z, const float angle = 0.0f): Qt3DCore::QEntity(root)
  {
    new Cylinder(this, 0.0f,  2.0f*l, 0.0f, -60.0f);
    new Cylinder(this, 0.0f, -2.0f*l, 0.0f, 0.0f);
    new Cylinder(this, 0.0f, -2.0f*l, 0.0f, 240.0f);
    new Sphere  (this, 0.0f,  2.0f*l, 0.0f);
    new Sphere  (this, 0.0f, -2.0f*l, 0.0f);
    Qt3DCore::QTransform* elementTransform = new Qt3DCore::QTransform();
    elementTransform->setRotationY(angle);
    elementTransform->setTranslation(QVector3D(x, y, z));
    addComponent(elementTransform);
  }
};
//-------------------------------------------------------------------------
class TopElement: public Qt3DCore::QEntity
{
public:
  TopElement(Qt3DCore::QEntity* root, const float x, const float y, const float z, const float angle): Qt3DCore::QEntity(root)
  {
    new Cylinder(this, 0.0f, 0.0f, 0.0f, 240.0f);
    new Sphere  (this, 0.0f, 0.0f, 0.0f);
    Qt3DCore::QTransform* elementTransform = new Qt3DCore::QTransform();
    elementTransform->setTranslation(QVector3D(x, y, z));
    elementTransform->setRotationY(angle);
    addComponent(elementTransform);
  }
};
//-------------------------------------------------------------------------
class BottomElement: public Qt3DCore::QEntity
{
public:
  BottomElement(Qt3DCore::QEntity* root, const float x, const float y, const float z, const float angle): Qt3DCore::QEntity(root)
  {
    new Cylinder(this, 0.0f, 0.0f, 0.0f, -60.0f);
    new Sphere  (this, 0.0f, 0.0f, 0.0f);
    Qt3DCore::QTransform* elementTransform = new Qt3DCore::QTransform();
    elementTransform->setTranslation(QVector3D(x, y, z));
    elementTransform->setRotationY(angle);
    addComponent(elementTransform);
  }
};
//-------------------------------------------------------------------------
class Molecule: public Qt3DCore::QEntity
{
public:
  Molecule(Qt3DCore::QEntity* root): Qt3DCore::QEntity(root)
  {
    const float cos30 = cos(30.0f);
    const int faces = 30; // количество граней может быть только чётным
    const float ANGLE = 360.0f / static_cast<float>(faces);
    const int height = 10;

    float x = 0.0f;
    float z = 0.0f;
    for (int i=1; i < faces; i = i + 2)
    {
      new TopElement   (this, 4.0f*l*cos30*x, 10.0f*l + 12.0f*l*static_cast<float>(height-1), -4.0f*l*cos30*z, static_cast<float>(i  )*ANGLE);
      for (int j=0; j < height; j = j + 1)
        new MiddleElement(this, 4.0f*l*cos30*x,  0.0f*l + 12.0f*l*static_cast<float>(j), -4.0f*l*cos30*z, static_cast<float>(i  )*ANGLE);
      x = x + cos(static_cast<float>(i  )*ANGLE);
      z = z + sin(static_cast<float>(i  )*ANGLE);
      for (int j=0; j < height; j = j + 1)
        new MiddleElement(this, 4.0f*l*cos30*x,  6.0f*l + 12.0f*l*static_cast<float>(j), -4.0f*l*cos30*z, static_cast<float>(i+1)*ANGLE);
      new BottomElement(this, 4.0f*l*cos30*x, -4.0f*l, -4.0f*l*cos30*z, static_cast<float>(i+1)*ANGLE);
      x = x + cos(static_cast<float>(i+1)*ANGLE);
      z = z + sin(static_cast<float>(i+1)*ANGLE);
    }
  }
};
//-------------------------------------------------------------------------
int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
  view->defaultFrameGraph()->setClearColor(QColor(QRgb(0xA0A0A0)));
  QWidget* container = QWidget::createWindowContainer(view);
  QSize screenSize = view->screen()->size();
  container->setMinimumSize(QSize(200, 100));
  container->setMaximumSize(screenSize);
  // Root entity
  Qt3DCore::QEntity* root = new Qt3DCore::QEntity();
  // Camera
  Qt3DRender::QCamera* camera = view->camera();
  camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
  camera->setPosition(QVector3D(0.0f, 0.0f, RADIUS));
  camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
  camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
  // For camera controls
  Qt3DExtras::QFirstPersonCameraController* camController = new Qt3DExtras::QFirstPersonCameraController(root);
  camController->setCamera(camera);
  // Light
  Qt3DCore::QEntity* light = new Qt3DCore::QEntity(root);
  Qt3DRender::QPointLight* pointLight = new Qt3DRender::QPointLight();
  pointLight->setColor("white");
  pointLight->setIntensity(1);
  light->addComponent(pointLight);
  Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform();
  lightTransform->setTranslation(camera->position());
  light->addComponent(lightTransform);

  QScrollBar* xBar = new QScrollBar(Qt::Horizontal);
  xBar->setMinimum(INDENT - RADIUS);
  xBar->setMaximum(RADIUS - INDENT);
  xBar->setValue(0);
  QScrollBar* yBar = new QScrollBar(Qt::Vertical);
  yBar->setMinimum(INDENT - RADIUS);
  yBar->setMaximum(RADIUS - INDENT);
  yBar->setValue(0);

  QGridLayout* gLayout = new QGridLayout();
  gLayout->addWidget(container, 0, 0);
  gLayout->addWidget(yBar, 0, 1);
  gLayout->addWidget(xBar, 1, 0);

  QWidget* window = new QWidget();
  QHBoxLayout *hLayout = new QHBoxLayout();
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setAlignment(Qt::AlignTop);
  hLayout->addLayout(gLayout, 1);
  hLayout->addLayout(vLayout, 0);
  window->setLayout(hLayout);

  new Molecule(root);

  QSpinBox* distance = new QSpinBox();
  distance->setMaximum(1000);
  distance->setValue(RADIUS);
  vLayout->addWidget(distance);
  QObject::connect(distance, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value){
    xBar->setMinimum(INDENT - value);
    xBar->setMaximum(value - INDENT);
    yBar->setMinimum(INDENT - value);
    yBar->setMaximum(value - INDENT);
    xBar->setValue(0);
    yBar->setValue(0);
    setCameraPos(camera, xBar->value(), yBar->value(), value);
  });
  QObject::connect(xBar, &QScrollBar::valueChanged, [=](int value){
    yBar->setValue(0);
    setCameraPos(camera, value, 0.0f, distance->value());
  });
  QObject::connect(yBar, &QScrollBar::valueChanged, [=](int value){
    xBar->setValue(0);
    setCameraPos(camera, 0.0f, value, distance->value());
  });

  view->setRootEntity(root);
  window->show();
  window->resize(1200, 800);
  return app.exec();
}
