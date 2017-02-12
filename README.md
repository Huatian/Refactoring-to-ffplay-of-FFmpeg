# Refactoring-to-ffplay-of-FFmpeg

#说明
本项目基于FFmpeg_3.0.5中的ffplay进行重构，有利于分析ffplay的整体流程和模块细节。<br>
项目删掉了filter相关的代码，项目中注释若有不当之处，尽请指正。<br>
联系邮箱：773512457@qq.com.

# 环境
  ubuntu 16.04LTS x86_64<br>
  ffmpeg 3.0.5<br>

# 使用
项目中有codeblocks的工程文件。可以将源码直接导入codeblocks。<br>
右键工程，properties -> Libraries ,然后你可以看到系统的共享库，将本工程需要的库关联起来。列表如下<br>
libavcodec<br>
libavformat<br>
libavutil<br>
libavdevice<br>
libavfilter<br>
libswscale<br>
libswresample<br>
libavresample<br>
libpostproc<br>
sdl<br>


# Reference & Thanks
  [ffmpeg](https://ffmpeg.org/)
