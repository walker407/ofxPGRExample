#include "FlyCapture2.h"

using namespace FlyCapture2;

class PGRCamera
{
	public:

		static const int camWidth  = 2080;
		static const int camHeight = 1552;

		Camera*   mCamera;
		ofImage*  mCamImage;

		PGRCamera()
		{

		}

		~PGRCamera()
		{
			mCamera->StopCapture();
			mCamera->Disconnect();

			delete mCamera;
			delete mCamImage;
		}

		void catchError(Error error) 
		{
			if(error != PGRERROR_OK)
				error.PrintErrorTrace();
		}

		void printInfo(const CameraInfo& cameraInfo) 
		{
			printf(
				"\n*** CAMERA INFORMATION ***\n"
				"Serial number - %u\n"
				"Camera model - %s\n"
				"Camera vendor - %s\n"
				"Sensor - %s\n"
				"Resolution - %s\n"
				"Firmware version - %s\n"
				"Firmware build time - %s\n\n",
				cameraInfo.serialNumber,
				cameraInfo.modelName,
				cameraInfo.vendorName,
				cameraInfo.sensorInfo,
				cameraInfo.sensorResolution,
				cameraInfo.firmwareVersion,
				cameraInfo.firmwareBuildTime);
		}

		void printBuildInfo()
		{
			FC2Version fc2Version;
			Utilities::GetLibraryVersion( &fc2Version );
			char version[128];
			sprintf(version,"FlyCapture2 library version: %d.%d.%d.%d\n",fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );  
				
			printf( "%s", version );

			char timeStamp[512];
			sprintf( timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );

			printf( "%s", timeStamp );

		}

		void printFormat7Capabilities(Format7Info fmt7Info)
		{
			 printf(
					"Max image pixels: (%u, %u)\n"
					"Image Unit size: (%u, %u)\n"
					"Offset Unit size: (%u, %u)\n"
					"Pixel format bitfield: 0x%08x\n",
					fmt7Info.maxWidth,
					fmt7Info.maxHeight,
					fmt7Info.imageHStepSize,
					fmt7Info.imageVStepSize,
					fmt7Info.offsetHStepSize,
					fmt7Info.offsetVStepSize,
					fmt7Info.pixelFormatBitField );

		}

		void setup() 
		{
			printBuildInfo();

			BusManager busMgr;

			unsigned int numCameras;

			catchError(busMgr.GetNumOfCameras(&numCameras));

			printf("Number of cameras detected: %u\n",numCameras);

			if(numCameras < 1)
			{
				printf("Insufficient number of cameras... exiting\n");
				return;
			}

			//Init Camera
			const Mode fmt7Mode = MODE_0;
			const PixelFormat fmt7PixFmt = PIXEL_FORMAT_MONO8;

			Camera* camera = new Camera();

			PGRGuid		guid;
			CameraInfo  camInfo;
			Format7Info fmt7Info;

			//Get First Connected Camera	
			catchError(busMgr.GetCameraFromIndex(0,&guid));
			
			//Connect to Camera
			catchError(camera->Connect(&guid));
			
			//Get Camera Information
			catchError(camera->GetCameraInfo(&camInfo));
			printInfo(camInfo);

			//Query Format 7 Modes
			bool supported;
			fmt7Info.mode = fmt7Mode;
			catchError(camera->GetFormat7Info(&fmt7Info,&supported));
			printFormat7Capabilities(fmt7Info);

			if ( (fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0 )
			{
				// Pixel format not supported!
				printf("Pixel format is not supported\n");
				return;
			}

			Format7ImageSettings fmt7ImageSettings;

			fmt7ImageSettings.mode			= fmt7Mode;
			fmt7ImageSettings.offsetX		= 0;
			fmt7ImageSettings.offsetY		= 0;
			fmt7ImageSettings.width			= fmt7Info.maxWidth;
			fmt7ImageSettings.height		= fmt7Info.maxHeight;
			fmt7ImageSettings.pixelFormat	= fmt7PixFmt;

			bool valid;
			Format7PacketInfo fmt7PacketInfo;

			//Validate Format 7 Settings
			catchError(camera->ValidateFormat7Settings(&fmt7ImageSettings,&valid,&fmt7PacketInfo));

			if(!valid)
			{
				printf("Format7 Settings are not Valid\n");
			}

			//Set Settings to Camera
			catchError(camera->SetFormat7Configuration(&fmt7ImageSettings,fmt7PacketInfo.recommendedBytesPerPacket));

			//Start Capture
			catchError(camera->StartCapture());

			//Allocate Image
			ofImage* image = new ofImage();
			image->allocate(camWidth,camHeight,OF_IMAGE_GRAYSCALE);
				
			//Init Camera & Image
			mCamera	   = camera;
			mCamImage  = image;
		}

		void update() 
		{
			Camera& camera = *mCamera;
			Image rawImage;

			catchError(camera.RetrieveBuffer(&rawImage));
			memcpy(mCamImage->getPixels(),rawImage.GetData(),camWidth*camHeight);

			//Flip Image
			mCamImage->mirror(true,false);

			mCamImage->update();
		}

		void draw() 
		{
			mCamImage->draw(0,0);
		}


};