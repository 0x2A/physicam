/*
	PhysiCam - Physically based camera
	Copyright (C) 2015 Frank Köhnke

	This file is part of PhysiCam.

	This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; either 
	version 3 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *	@file ShaderCode.cpp
 */

#include <physicam/ShaderCode.h>

namespace PhysiCam
{
	const static std::string ScreenAlignedVertSrc = R"(
	
	#version 400
	layout(location = 0) in vec3 vertexPosition;
	layout(location = 1) in vec3 vertexNormal;
	layout(location = 2) in vec2 vertexUV;
	layout(location = 3) in vec4 vertexColor;

	uniform mat4 MVPMatrix;

	out vec2 texCoord;

	void main(void)
	{
		texCoord = vertexUV;
		gl_Position = vec4(vertexPosition, 1);
	};
	)";

	const static std::string BlitScreenSrc = R"(

		#version 400
		uniform sampler2D tex;
		uniform float exp;

		in vec2 texCoord;

		out vec4 colorOut;
		
		void main(void)
		{
			colorOut = vec4(texture(tex,texCoord).xyz  * exp, 1);
		};

	)";

	const static std::string LensDistortionSrc = R"(

		/*
			Original Lens Distortion Algorithm from SSontech (Syntheyes)
			http://www.ssontech.com/content/lensalg.htm

			r2 = image_aspect*image_aspect*u*u + v*v
		*/

		#version 400
		uniform sampler2D tex;
		uniform sampler2D depth;
		uniform float k = 0.3;
		//uniform float kcube = 0.5;
		uniform float scale = 0.9;
		uniform float dispersion = 0.01;
		uniform float blurAmount = 0.5; //k = 0.2, kcube = 0.3, scale = 0.9, dispersion = 0.01
		uniform bool blurEnabled = false;
		uniform vec2 screenSize;

		in vec2 texCoord;

		layout(location = 0) out vec4 colorOut;
		layout(location = 1) out vec4 depthOut;

		vec2 rand(vec2 co) //needed for fast noise based blurring
		{
			float noise1 =  (fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453));
			float noise2 =  (fract(sin(dot(co ,vec2(12.9898,78.233)*2.0)) * 43758.5453));
			return clamp(vec2(noise1,noise2),0.0,1.0);
		}
		
		void main(void)
		{
			//index of refraction of each color channel, causing chromatic dispersion
			vec3 eta = vec3(1.0+dispersion*0.9, 1.0+dispersion*0.6, 1.0+dispersion*0.3);

		
			float aspect = screenSize.x / screenSize.y;
			//float r2 = aspect * aspect * texCoord.x * texCoord.x * texCoord.y * texCoord.y;
			float r2 = (texCoord.x-0.5) * (texCoord.x-0.5) + (texCoord.y-0.5) * (texCoord.y-0.5);
			float f;
			//if( kcube == 0.0)
			//{
				f = 1.0 + r2 * k;
			//}else{
				//f = 1.0 + r2 * (k + kcube * sqrt(r2));
			//};

			// get the right pixel for the current position
			vec2 rCoords = (f*eta.r)*scale*(texCoord.xy-0.5)+0.5;
			vec2 gCoords = (f*eta.g)*scale*(texCoord.xy-0.5)+0.5;
			vec2 bCoords = (f*eta.b)*scale*(texCoord.xy-0.5)+0.5;
			
			vec3 inputDistort = vec3(0.0); 
			inputDistort.r = texture(tex,rCoords).r;
			inputDistort.g = texture(tex,gCoords).g;
			inputDistort.b = texture(tex,bCoords).b;
			
			colorOut = vec4(inputDistort.r,inputDistort.g,inputDistort.b,1.0);
			depthOut = vec4(texture(depth, rCoords).r);
		};

	)";

	const static std::string DownsampleScreenSrc = R"(

		#version 400
		uniform sampler2D tex;
		in vec2 texCoord;

		out vec4 colorOut;
		
		void main(void)
		{
			colorOut = vec4(texture(tex,texCoord).rgb, 1);
		};

	)";

	const static std::string BrightPassSrc = R"(
		
		#version 400

		uniform sampler2D tex;
		uniform float threshold;
		uniform float LenseFlareThreshold;
		uniform float exp;

		in vec2 texCoord;
	
		layout(location = 0) out vec4 BrightColor;
		layout(location = 1) out vec4 LenseFlareBrightColor;

		void main(void)
		{
			vec4 color = vec4(texture(tex,texCoord).xyz,1);
			float luminance = (0.2126f*color.x + 0.7152f*color.y + 0.0722f*color.z);

			if(luminance > threshold)
				BrightColor = color;
			else
				BrightColor = vec4(0,0,0,1);

			if(luminance > LenseFlareThreshold)
				LenseFlareBrightColor = color  / (LenseFlareThreshold-1);
			else
				LenseFlareBrightColor = vec4(0,0,0,1);
		};

	)";

	const static std::string BlurHorizontalSrc = R"(
		
		#version 400

		uniform sampler2D tex;
		uniform float radius;
		uniform float resolution;

		in vec2 texCoord;

		out vec4 colorOut;

		void main(void)
		{
			//our original texcoord for this fragment
		    vec2 tc = texCoord;

			//the amount to blur, i.e. how far off center to sample from
			float blur = radius/resolution; 

			vec4 sum = vec4(0.0);

			//apply blurring, using a 9-tap filter with predefined gaussian weights
			sum += texture2D(tex, vec2(tc.x - 4.0*blur, tc.y)) * 0.0162162162;
			sum += texture2D(tex, vec2(tc.x - 3.0*blur, tc.y)) * 0.0540540541;
			sum += texture2D(tex, vec2(tc.x - 2.0*blur, tc.y)) * 0.1216216216;
			sum += texture2D(tex, vec2(tc.x - 1.0*blur, tc.y)) * 0.1945945946;
			sum += texture2D(tex, vec2(tc.x, tc.y)) * 0.2270270270;
			sum += texture2D(tex, vec2(tc.x + 1.0*blur, tc.y)) * 0.1945945946;
			sum += texture2D(tex, vec2(tc.x + 2.0*blur, tc.y)) * 0.1216216216;
			sum += texture2D(tex, vec2(tc.x + 3.0*blur, tc.y)) * 0.0540540541;
			sum += texture2D(tex, vec2(tc.x + 4.0*blur, tc.y)) * 0.0162162162;

			colorOut = vec4(sum.xyz,1);
		};

	)";

	const static std::string BlurVerticalSrc = R"(
		
		#version 400

		uniform sampler2D tex;
		uniform float radius;
		uniform float resolution;

		in vec2 texCoord;

		out vec4 colorOut;

		void main(void)
		{
			//our original texcoord for this fragment
		    vec2 tc = texCoord;

			//the amount to blur, i.e. how far off center to sample from
			float blur = radius/resolution; 

			vec4 sum = vec4(0.0);

			//apply blurring, using a 9-tap filter with predefined gaussian weights
			sum += texture2D(tex, vec2(tc.x, tc.y - 4.0*blur)) * 0.0162162162;
			sum += texture2D(tex, vec2(tc.x, tc.y - 3.0*blur)) * 0.0540540541;
			sum += texture2D(tex, vec2(tc.x, tc.y - 2.0*blur)) * 0.1216216216;
			sum += texture2D(tex, vec2(tc.x, tc.y - 1.0*blur)) * 0.1945945946;
			sum += texture2D(tex, vec2(tc.x, tc.y)) * 0.2270270270;
			sum += texture2D(tex, vec2(tc.x, tc.y + 1.0*blur)) * 0.1945945946;
			sum += texture2D(tex, vec2(tc.x, tc.y + 2.0*blur)) * 0.1216216216;
			sum += texture2D(tex, vec2(tc.x, tc.y + 3.0*blur)) * 0.0540540541;
			sum += texture2D(tex, vec2(tc.x, tc.y + 4.0*blur)) * 0.0162162162;

			colorOut = vec4(sum.xyz,1);
		};

	)";

	const static std::string IncrGaussBlurSrc = R"(
		
		#version 400
		#define MAX_BLUR_RADIUS 4096

		uniform sampler2D tex;
		uniform float radius;
		uniform vec2 uBlurDirection;	// (1,0)/(0,1) for x/y pass
		uniform vec2 resolution;

		in vec2 texCoord;

		out vec4 colorOut;

		/*----------------------------------------------------------------------------*/
		/*	Incremental, forward-differencing Gaussian elimination based on:
			http://http.developer.nvidia.com/GPUGems3/gpugems3_ch40.html */
		vec4 incrementalGauss1D(
			in sampler2D srcTex, 
			in vec2 srcTexelSize, 
			in vec2 origin,
			in float radius,
			in vec2 direction
		) {

			int nSamples = clamp(int(radius), 1, int(MAX_BLUR_RADIUS)) / 2;
	
			if (nSamples == 0)
				return texture(srcTex, origin);
	
			float SIGMA = radius / 8.0;
			float sig2 = SIGMA * SIGMA;
			const float TWO_PI	= 6.2831853071795;
			const float E			= 2.7182818284590;
		
		//	set up incremental counter:
			vec3 gaussInc;
			gaussInc.x = 1.0 / (sqrt(TWO_PI) * SIGMA);
			gaussInc.y = exp(-0.5 / sig2);
			gaussInc.z = gaussInc.y * gaussInc.y;
	
		//	accumulate results:
			vec4 result = texture(srcTex, origin) * gaussInc.x;	
			for (int i = 1; i < nSamples; ++i) {
				gaussInc.xy *= gaussInc.yz;
		
				vec2 offset = float(i) * direction * srcTexelSize;
				result += texture(srcTex, origin - offset) * gaussInc.x;
				result += texture(srcTex, origin + offset) * gaussInc.x;
			}
	
			return result;
		}

		void main(void)
		{
			vec2 texelSize = 1.0 / resolution;
			colorOut = incrementalGauss1D(tex, texelSize, texCoord, radius, uBlurDirection);
		};

	)";

	const static std::string BloomComposeSrc = R"(
		
		#version 400

		uniform sampler2D tex[5];
		uniform float strengths[5];
		uniform float intensity;

		in vec2 texCoord;

		out vec4 colorOut;

		void main(void)
		{
			vec4 sum = texture(tex[0], texCoord)*strengths[0];
			sum += texture(tex[1], texCoord)*strengths[1];//*0.2;
			sum += texture(tex[2], texCoord)*strengths[2];
			sum += texture(tex[3], texCoord)*strengths[3];
			sum += texture(tex[4], texCoord)*strengths[4];
			sum *= intensity;
			
			colorOut = vec4(sum.xyz,1);
		};

	)";

	const static std::string BloomLenseComposeSrc = R"(
		
		#version 400

		uniform sampler2D bloomPass;
		uniform sampler2D lenseFlare;
		uniform sampler2D baseTex;
		uniform sampler2D dirtTexture;

		uniform int hasDirtTexture;
		uniform float strength;

		in vec2 texCoord;

		out vec4 colorOut;

		void main(void)
		{
			vec4 bloom = texture(bloomPass, texCoord);
			vec4 base = texture(baseTex, texCoord);
			vec4 lense = texture(lenseFlare, texCoord)*strength;

			if(hasDirtTexture > 0)
			{
				vec4 dirt = texture(dirtTexture, texCoord);
				bloom *= dirt;
				lense *= dirt;
			}
			
			colorOut = vec4(bloom.xyz+lense.xyz+base.xyz,1);
		};

	)";


	const static std::string LenseFlareSrc = R"(
		
		#version 400

		uniform sampler2D tex;
		uniform vec2 screenSize;
		uniform float HaloWidth;
		uniform vec3 ChromaticDistortionVector;

		in vec2 texCoord;

		out vec4 colorOut;
		
		const int uSamples = 8;
		const float uDispersal = 0.3;
		const float uDistortion = 1.0;
		
		// chromatic distortion:
		vec4 textureDistorted(in sampler2D tex, in vec2 texcoord,
							  in vec2 direction, in vec3 distortion)
		{
		  return vec4(texture(tex, texcoord + direction * distortion.r).r,
						texture(tex, texcoord + direction * distortion.g).g,
					  texture(tex, texcoord + direction * distortion.b).b, 1.0);
		}

		void main(void)
		{
			vec2 texcoord = -texCoord + vec2(1.0); // flip texcoordoords
			vec2 texelSize = 1.0 / screenSize;

			vec2 ghostVec = (vec2(0.5) - texcoord) * uDispersal;
			vec2 haloVec = normalize(ghostVec) * HaloWidth;

			vec3 distortion = vec3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);
			vec4 result = vec4(0.0);
			for (int i = 0; i < uSamples; ++i) 
			{
				vec2 offset = fract(texcoord + ghostVec * float(i));
		
				float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
				weight = pow(1.0 - weight, 10.0);
	
				result += textureDistorted(
					tex,
					offset,
					normalize(ghostVec),
					distortion
				) * weight;
			}

			//	sample halo:
			float weight = length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));
			weight = pow(1.0 - weight, 10.0);
			result += textureDistorted(
				tex,
				fract(texcoord + haloVec),
				normalize(ghostVec),
				distortion
			) * weight;
			colorOut = vec4(result.xyz,1);
		};

	)";

	const static std::string ToneMapperSrc = R"(
		
		#version 400

		uniform sampler2D hdrColor;
		uniform int tonemappingMethod;
		uniform bool tonemappingEnabled = true;
		uniform bool noiseEnabled = true;
		uniform float timer;
		uniform float grainamount;
		uniform vec2 screenSize;

		in vec2 texCoord;
		out lowp vec4 colorOut;

		float A = 0.15;
		float B = 0.50;
		float C = 0.10;
		float D = 0.20;
		float E = 0.02;
		float F = 0.30;
		float W = 11.2;

		const float permTexUnit = 1.0/256.0;		// Perm texture texel-size
		const float permTexUnitHalf = 0.5/256.0;	// Half perm texture texel-size

		bool colored = false;
		float coloramount = 0.6;
		float grainsize = 1.6;
		float lumamount = 1.0;

		vec3 Reinhard(vec3 col)
		{
			vec3 mapped = col / (col + vec3(1.0));
			// Gamma correction 
			mapped = pow(mapped, vec3(1.0 / 2.2));
			return mapped;
		}

		vec3 Filmic(vec3 col)
		{
			vec3 x = max(vec3(0.0), col-0.004);
			return (x*(6.2*x+0.5))/(x*(6.2*x+1.7)+0.06);
		}

														
		vec3 Uncharted2Tonemap(vec3 x)
		{
		   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
		}
				
		//a random texture generator, but you can also use a pre-computed perturbation texture
		vec4 rnm(in vec2 tc) 
		{
			float noise =  sin(dot(tc + vec2(timer,timer),vec2(12.9898,78.233))) * 43758.5453;

			float noiseR =  fract(noise)*2.0-1.0;
			float noiseG =  fract(noise*1.2154)*2.0-1.0; 
			float noiseB =  fract(noise*1.3453)*2.0-1.0;
			float noiseA =  fract(noise*1.3647)*2.0-1.0;
	
			return vec4(noiseR,noiseG,noiseB,noiseA);
		}

		float fade(in float t) {
			return t*t*t*(t*(t*6.0-15.0)+10.0);
		}

						
		float pnoise3D(in vec3 p)
		{
			vec3 pi = permTexUnit*floor(p)+permTexUnitHalf; // Integer part, scaled so +1 moves permTexUnit texel
			// and offset 1/2 texel to sample texel centers
			vec3 pf = fract(p);     // Fractional part for interpolation

			// Noise contributions from (x=0, y=0), z=0 and z=1
			float perm00 = rnm(pi.xy).a ;
			vec3  grad000 = rnm(vec2(perm00, pi.z)).rgb * 4.0 - 1.0;
			float n000 = dot(grad000, pf);
			vec3  grad001 = rnm(vec2(perm00, pi.z + permTexUnit)).rgb * 4.0 - 1.0;
			float n001 = dot(grad001, pf - vec3(0.0, 0.0, 1.0));

			// Noise contributions from (x=0, y=1), z=0 and z=1
			float perm01 = rnm(pi.xy + vec2(0.0, permTexUnit)).a ;
			vec3  grad010 = rnm(vec2(perm01, pi.z)).rgb * 4.0 - 1.0;
			float n010 = dot(grad010, pf - vec3(0.0, 1.0, 0.0));
			vec3  grad011 = rnm(vec2(perm01, pi.z + permTexUnit)).rgb * 4.0 - 1.0;
			float n011 = dot(grad011, pf - vec3(0.0, 1.0, 1.0));

			// Noise contributions from (x=1, y=0), z=0 and z=1
			float perm10 = rnm(pi.xy + vec2(permTexUnit, 0.0)).a ;
			vec3  grad100 = rnm(vec2(perm10, pi.z)).rgb * 4.0 - 1.0;
			float n100 = dot(grad100, pf - vec3(1.0, 0.0, 0.0));
			vec3  grad101 = rnm(vec2(perm10, pi.z + permTexUnit)).rgb * 4.0 - 1.0;
			float n101 = dot(grad101, pf - vec3(1.0, 0.0, 1.0));

			// Noise contributions from (x=1, y=1), z=0 and z=1
			float perm11 = rnm(pi.xy + vec2(permTexUnit, permTexUnit)).a ;
			vec3  grad110 = rnm(vec2(perm11, pi.z)).rgb * 4.0 - 1.0;
			float n110 = dot(grad110, pf - vec3(1.0, 1.0, 0.0));
			vec3  grad111 = rnm(vec2(perm11, pi.z + permTexUnit)).rgb * 4.0 - 1.0;
			float n111 = dot(grad111, pf - vec3(1.0, 1.0, 1.0));

			// Blend contributions along x
			vec4 n_x = mix(vec4(n000, n001, n010, n011), vec4(n100, n101, n110, n111), fade(pf.x));

			// Blend contributions along y
			vec2 n_xy = mix(n_x.xy, n_x.zw, fade(pf.y));

			// Blend contributions along z
			float n_xyz = mix(n_xy.x, n_xy.y, fade(pf.z));

			// We're done, return the final noise value.
			return n_xyz;
		}

		//2d coordinate orientation thing
		vec2 coordRot(in vec2 tc, in float angle)
		{
			float aspect = screenSize.x/screenSize.y;
			float rotX = ((tc.x*2.0-1.0)*aspect*cos(angle)) - ((tc.y*2.0-1.0)*sin(angle));
			float rotY = ((tc.y*2.0-1.0)*cos(angle)) + ((tc.x*2.0-1.0)*aspect*sin(angle));
			rotX = ((rotX/aspect)*0.5+0.5);
			rotY = rotY*0.5+0.5;
			return vec2(rotX,rotY);
		}

		vec3 Noise(vec3 col)
		{
			const vec3 rotOffset = vec3(1.425,3.892,5.835); //rotation offset values	
			vec2 rotCoordsR = coordRot(texCoord, timer + rotOffset.x);
			vec3 noise = vec3(pnoise3D(vec3(rotCoordsR*vec2(screenSize.x/grainsize,screenSize.y/grainsize),0.0)));

			//colored noise
			//vec2 rotCoordsG = coordRot(texCoord, timer + rotOffset.y);
			//vec2 rotCoordsB = coordRot(texCoord, timer + rotOffset.z);
			//noise.g = mix(noise.r,pnoise3D(vec3(rotCoordsG*vec2(screenSize.x/grainsize,screenSize.y/grainsize),1.0)),coloramount);
			//noise.b = mix(noise.r,pnoise3D(vec3(rotCoordsB*vec2(screenSize.x/grainsize,screenSize.y/grainsize),2.0)),coloramount);

			//noisiness response curve based on scene luminance
			const vec3 lumcoeff = vec3(0.299,0.587,0.114);
			float luminance = mix(0.0,dot(col, lumcoeff),lumamount);
			float lum = smoothstep(0.2,0.0,luminance);
			lum += luminance;

			noise = mix(noise,vec3(0.0),luminance);

			return noise;
		}

		void main(void)
		{
			vec3 color = texture(hdrColor, texCoord).xyz;
			if(tonemappingEnabled)
			{
				switch(tonemappingMethod)
				{
					case 0:
						color = Reinhard(color);
						break;
					case 1:
						color = Filmic(color);
						break;
					case 2:
						{
							float ExposureBias = 2.0f;
							color = Uncharted2Tonemap(ExposureBias*color);
							vec3 whiteScale = vec3(1.0f)/Uncharted2Tonemap(vec3(W));
							color = color * whiteScale;
							color = pow(color, vec3(1/2.2));
						}
						break;
				}
			}
			if(noiseEnabled)
			{
				color += Noise(color)*grainamount;
			}

			colorOut = vec4(color,1);
		};


		)";

	const static std::string DoFSrc = R"(

				#version 400
		#define PI  3.14159265

				uniform sampler2D ColorTexture;
		uniform sampler2D DepthTexture;
		uniform vec2 ScreenSize;
				
		uniform bool autofocus;
		uniform float focalDepth;  //focal distance value in meters, but you may use autofocus option below
		uniform float focalLength; //focal length in mm
		uniform float fstop; //f-stop value
		uniform float fringe; //bokeh chromatic aberration/fringing
		uniform bool vignetting;
		uniform float maxblur; //clamp value of max blur (0.0 = no blur,1.0 default)
		uniform bool showFocus; //show debug focus point and focal range (red = focal point, green = focal range)
		uniform float CoC; //circle of confusion size in mm (35mm film = 0.03mm)

		uniform vec2 CameraClips;		

		in vec2 texCoord;

		out vec4 colorOut;

		float width = ScreenSize.x; //texture width
		float height = ScreenSize.y; //texture height

		vec2 texel = vec2(1.0/width,1.0/height);
	

		//user variables ----

		int samples = 6; //samples on the first ring
		int rings = 3; //ring count

		
		 //use optical lens vignetting?
		float vignout = 1.3; //vignetting outer border
		float vignin = 0.0; //vignetting inner border
		float vignfade = 22.0; //f-stops till vignete fades

		 //use autofocus in shader? disable if you use external focalDepth value
		vec2 focus = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)		

		float threshold = 1.0; //highlight threshold;
		float gain = 1.8; //highlight gain;

		float bias = 0.5; //bokeh edge bias
		
		bool noise = true; //use noise instead of pattern for sample dithering
		float namount = 0.0001; //dither amount

		bool depthblur = false; //blur the depth buffer?
		float dbsize = 1.25; //depthblursize

		bool pentagon = false; //use pentagon as bokeh shape?
		float feather = 0.4; //pentagon shape feather

						
		const vec3 lumcoeff = vec3(0.299,0.587,0.114);

		float bdepth(vec2 coords) //blurring depth
		{
			float d = 0.0;
			float kernel[9];
			vec2 offset[9];
	
			vec2 wh = vec2(texel.x, texel.y) * dbsize;
	
			offset[0] = vec2(-wh.x,-wh.y);
			offset[1] = vec2( 0.0, -wh.y);
			offset[2] = vec2( wh.x -wh.y);
	
			offset[3] = vec2(-wh.x,  0.0);
			offset[4] = vec2( 0.0,   0.0);
			offset[5] = vec2( wh.x,  0.0);
	
			offset[6] = vec2(-wh.x, wh.y);
			offset[7] = vec2( 0.0,  wh.y);
			offset[8] = vec2( wh.x, wh.y);
	
			kernel[0] = 1.0/16.0;   kernel[1] = 2.0/16.0;   kernel[2] = 1.0/16.0;
			kernel[3] = 2.0/16.0;   kernel[4] = 4.0/16.0;   kernel[5] = 2.0/16.0;
			kernel[6] = 1.0/16.0;   kernel[7] = 2.0/16.0;   kernel[8] = 1.0/16.0;
	
	
			for( int i=0; i<9; i++ )
			{
				float tmp = texture2D(DepthTexture, coords + offset[i]).r;
				d += tmp * kernel[i];
			}
	
			return d;
		}

		vec2 rand(in vec2 coord) //generating noise/pattern texture for dithering
		{
			float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
			float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	
			if (noise)
			{
				noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
				noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
			}
			return vec2(noiseX,noiseY);
		}

		vec3 color(vec2 coords, float blur) //processing the sample
		{
			vec3 col = vec3(0.0);
	
			col.r = texture2D(ColorTexture,coords + vec2(0.0,1.0)*texel*fringe*blur).r;
			col.g = texture2D(ColorTexture,coords + vec2(-0.866,-0.5)*texel*fringe*blur).g;
			col.b = texture2D(ColorTexture,coords + vec2(0.866,-0.5)*texel*fringe*blur).b;
	
			
			float lum = dot(col.rgb, lumcoeff);
			float thresh = max((lum-threshold)*gain, 0.0);
			return col+mix(vec3(0.0),col,thresh*blur);
		}

		float penta(vec2 coords) //pentagonal shape
		{
			float scale = float(rings) - 1.3;
			vec4  HS0 = vec4( 1.0,         0.0,         0.0,  1.0);
			vec4  HS1 = vec4( 0.309016994, 0.951056516, 0.0,  1.0);
			vec4  HS2 = vec4(-0.809016994, 0.587785252, 0.0,  1.0);
			vec4  HS3 = vec4(-0.809016994,-0.587785252, 0.0,  1.0);
			vec4  HS4 = vec4( 0.309016994,-0.951056516, 0.0,  1.0);
			vec4  HS5 = vec4( 0.0        ,0.0         , 1.0,  1.0);
	
			vec4  one = vec4( 1.0 );
	
			vec4 P = vec4((coords),vec2(scale, scale)); 
	
			vec4 dist = vec4(0.0);
			float inorout = -4.0;
	
			dist.x = dot( P, HS0 );
			dist.y = dot( P, HS1 );
			dist.z = dot( P, HS2 );
			dist.w = dot( P, HS3 );
	
			dist = smoothstep( -feather, feather, dist );
	
			inorout += dot( dist, one );
	
			dist.x = dot( P, HS4 );
			dist.y = HS5.w - abs( P.z );
	
			dist = smoothstep( -feather, feather, dist );
			inorout += dist.x;
	
			return clamp( inorout, 0.0, 1.0 );
		}

		vec3 debugFocus(vec3 col, float blur, float depth)
		{
			float edge = 0.002*depth; //distance based edge smoothing
			float m = clamp(smoothstep(0.0,edge,blur),0.0,1.0);
			float e = clamp(smoothstep(1.0-edge,1.0,blur),0.0,1.0);
	
			col = mix(col,vec3(1.0,0.5,0.0),(1.0-m)*0.6);
			col = mix(col,vec3(0.0,0.5,1.0),((1.0-e)-(1.0-m))*0.2);

				return col;
		}
		
		float linearize(float depth)
		{
			return -CameraClips.y * CameraClips.x / (depth * (CameraClips.y - CameraClips.x) - CameraClips.y);
		}

		float vignette()
		{
			float dist = distance(texCoord, vec2(0.5,0.5));
			dist = smoothstep(vignout+(fstop/vignfade), vignin+(fstop/vignfade), dist);
			return clamp(dist,0.0,1.0);
		}

		void main(void)
		{
			float depth = 0.0;
			if (depthblur)
				depth = linearize(bdepth(texCoord));
			else
				depth = linearize(texture2D(DepthTexture,texCoord).x);

			//focal plane calculation
			float fDepth = focalDepth;
			if (autofocus)
			{
				fDepth = linearize(texture2D(DepthTexture,focus).x);
			}

			float f = focalLength; //focal length in mm
			float d = fDepth*1000.0; //focal plane in mm
			float o = depth*1000.0; //depth in mm

			float a = (o*f)/(o-f); 
			float b = (d*f)/(d-f); 
			float c = (d-f)/(d*fstop*CoC); 
			float blur = abs(a-b)*c;
			blur = clamp(blur,0.0,1.0);

			vec2 noise = rand(texCoord)*namount*blur;

			// getting blur x and y step factor
			float w = (1.0/width)*blur*maxblur+noise.x;
			float h = (1.0/height)*blur*maxblur+noise.y;

			vec3 col = vec3(0.0);
			if(blur < 0.05) //some optimization thingy
				col = texture2D(ColorTexture, texCoord).rgb;
			else
			{
				col = texture2D(ColorTexture, texCoord).rgb;
				float s = 1.0;
				int ringsamples;
		
				for (int i = 1; i <= rings; i += 1)
				{   
					ringsamples = i * samples;
			
					for (int j = 0 ; j < ringsamples ; j += 1)   
					{
						float step = PI*2.0 / float(ringsamples);
						float pw = (cos(float(j)*step)*float(i));
						float ph = (sin(float(j)*step)*float(i));
						float p = 1.0;
						if (pentagon)
						{ 
							p = penta(vec2(pw,ph));
						}
						col += color(texCoord + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(rings)),bias)*p;  
						s += 1.0*mix(1.0,(float(i))/(float(rings)),bias)*p;   
					}
				}
				col /= s; //divide by sample count
			}

			if (showFocus)
			{
				col = debugFocus(col, blur, depth);
			}
			if (vignetting)
			{
				col *= vignette();
			}
			colorOut = vec4(col,1);
		};

	)";

	/*const static std::string LenseFlareSrc = R"(
		
		#version 400

		uniform sampler2D tex;
		uniform int ghosts; // number of ghost samples
		uniform float ghostDispersal; // dispersion factor
		uniform vec2 screenSize;

		in vec2 texCoord;

		out vec4 colorOut;

		void main(void)
		{
			vec2 texcoord = -texCoord + vec2(1.0);
			vec2 texelSize = 1.0 / screenSize;
	
			// ghost vector to image centre:
			vec2 ghostVec = (vec2(0.5) - texcoord) * ghostDispersal;

			// sample ghosts:  
			vec4 result = vec4(0.0);
			float pers = 1.0 / ghosts;
			for (int i = 0; i < ghosts; ++i) 
			{ 
				vec2 offset = texcoord + ghostVec * float(i);
				
				float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
				weight = clamp(pow(1.0 - weight, 15.0),0,1);

				result += clamp(texture(tex, offset),0,1) * weight * pers;
			}

			colorOut = vec4(result.xyz,1);
		};

	)";*/
}