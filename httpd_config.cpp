

#include "httpd_config.h"
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>

namespace simple_httpd {

static HttpdConfig * s_instance = NULL;

HttpdConfig::HttpdConfig()
{
    mMetaMap[".*"] = "application/octet-stream";
    mMetaMap[".tif"] = "image/tiff";
    mMetaMap[".001"] = "application/x-001";
    mMetaMap[".301"] = "application/x-301";
    mMetaMap[".323"] = "text/h323";
    mMetaMap[".906"] = "application/x-906";
    mMetaMap[".907"] = "drawing/907";
    mMetaMap[".a11"] = "application/x-a11";
    mMetaMap[".acp"] = "audio/x-mei-aac";
    mMetaMap[".ai"] = "application/postscript";
    mMetaMap[".aif"] = "audio/aiff";
    mMetaMap[".aifc"] = "audio/aiff";
    mMetaMap[".aiff"] = "audio/aiff";
    mMetaMap[".anv"] = "application/x-anv";
    mMetaMap[".asa"] = "text/asa";
    mMetaMap[".asf"] = "video/x-ms-asf";
    mMetaMap[".asp"] = "text/asp";
    mMetaMap[".asx"] = "video/x-ms-asf";
    mMetaMap[".au"] = "audio/basic";
    mMetaMap[".avi"] = "video/avi";
    mMetaMap[".awf"] = "application/vnd.adobe.workflow";
    mMetaMap[".biz"] = "text/xml";
    mMetaMap[".bmp"] = "application/x-bmp";
    mMetaMap[".bot"] = "application/x-bot";
    mMetaMap[".c4t"] = "application/x-c4t";
    mMetaMap[".c90"] = "application/x-c90";
    mMetaMap[".cal"] = "application/x-cals";
    mMetaMap[".cat"] = "application/vnd.ms-pki.seccat";
    mMetaMap[".cdf"] = "application/x-netcdf";
    mMetaMap[".cdr"] = "application/x-cdr";
    mMetaMap[".cel"] = "application/x-cel";
    mMetaMap[".cer"] = "application/x-x509-ca-cert";
    mMetaMap[".cg4"] = "application/x-g4";
    mMetaMap[".cgm"] = "application/x-cgm";
    mMetaMap[".cit"] = "application/x-cit";
    mMetaMap[".class"] = "java/*";
    mMetaMap[".cml"] = "text/xml";
    mMetaMap[".cmp"] = "application/x-cmp";
    mMetaMap[".cmx"] = "application/x-cmx";
    mMetaMap[".cot"] = "application/x-cot";
    mMetaMap[".crl"] = "application/pkix-crl";
    mMetaMap[".crt"] = "application/x-x509-ca-cert";
    mMetaMap[".csi"] = "application/x-csi";
    mMetaMap[".css"] = "text/css";
    mMetaMap[".cut"] = "application/x-cut";
    mMetaMap[".dbf"] = "application/x-dbf";
    mMetaMap[".dbm"] = "application/x-dbm";
    mMetaMap[".dbx"] = "application/x-dbx";
    mMetaMap[".dcd"] = "text/xml";
    mMetaMap[".dcx"] = "application/x-dcx";
    mMetaMap[".der"] = "application/x-x509-ca-cert";
    mMetaMap[".dgn"] = "application/x-dgn";
    mMetaMap[".dib"] = "application/x-dib";
    mMetaMap[".dll"] = "application/x-msdownload";
    mMetaMap[".doc"] = "application/msword";
    mMetaMap[".dot"] = "application/msword";
    mMetaMap[".drw"] = "application/x-drw";
    mMetaMap[".dtd"] = "text/xml";
    mMetaMap[".dwf"] = "Model/vnd.dwf";
    mMetaMap[".dwf"] = "application/x-dwf";
    mMetaMap[".dwg"] = "application/x-dwg";
    mMetaMap[".dxb"] = "application/x-dxb";
    mMetaMap[".dxf"] = "application/x-dxf";
    mMetaMap[".edn"] = "application/vnd.adobe.edn";
    mMetaMap[".emf"] = "application/x-emf";
    mMetaMap[".eml"] = "message/rfc822";
    mMetaMap[".ent"] = "text/xml";
    mMetaMap[".epi"] = "application/x-epi";
    mMetaMap[".eps"] = "application/x-ps";
    mMetaMap[".eps"] = "application/postscript";
    mMetaMap[".etd"] = "application/x-ebx";
    mMetaMap[".exe"] = "application/x-msdownload";
    mMetaMap[".fax"] = "image/fax";
    mMetaMap[".fdf"] = "application/vnd.fdf";
    mMetaMap[".fif"] = "application/fractals";
    mMetaMap[".fo"] = "text/xml";
    mMetaMap[".frm"] = "application/x-frm";
    mMetaMap[".g4"] = "application/x-g4";
    mMetaMap[".gbr"] = "application/x-gbr";
    mMetaMap["."] = "application/x-";
    mMetaMap[".gif"] = "image/gif";
    mMetaMap[".gl2"] = "application/x-gl2";
    mMetaMap[".gp4"] = "application/x-gp4";
    mMetaMap[".hgl"] = "application/x-hgl";
    mMetaMap[".hmr"] = "application/x-hmr";
    mMetaMap[".hpg"] = "application/x-hpgl";
    mMetaMap[".hpl"] = "application/x-hpl";
    mMetaMap[".hqx"] = "application/mac-binhex40";
    mMetaMap[".hrf"] = "application/x-hrf";
    mMetaMap[".hta"] = "application/hta";
    mMetaMap[".htc"] = "text/x-component";
    mMetaMap[".htm"] = "text/html";
    mMetaMap[".html"] = "text/html";
    mMetaMap[".htt"] = "text/webviewhtml";
    mMetaMap[".htx"] = "text/html";
    mMetaMap[".icb"] = "application/x-icb";
    mMetaMap[".ico"] = "image/x-icon";
    mMetaMap[".ico"] = "application/x-ico";
    mMetaMap[".iff"] = "application/x-iff";
    mMetaMap[".ig4"] = "application/x-g4";
    mMetaMap[".igs"] = "application/x-igs";
    mMetaMap[".iii"] = "application/x-iphone";
    mMetaMap[".img"] = "application/x-img";
    mMetaMap[".ins"] = "application/x-internet-signup";
    mMetaMap[".isp"] = "application/x-internet-signup";
    mMetaMap[".IVF"] = "video/x-ivf";
    mMetaMap[".java"] = "java/*";
    mMetaMap[".jfif"] = "image/jpeg";
    mMetaMap[".jpe"] = "image/jpeg";
    mMetaMap[".jpe"] = "application/x-jpe";
    mMetaMap[".jpeg"] = "image/jpeg";
    mMetaMap[".jpg"] = "image/jpeg";
    mMetaMap[".jpg"] = "application/x-jpg";
    mMetaMap[".js"] = "application/x-javascript";
    mMetaMap[".jsp"] = "text/html";
    mMetaMap[".la1"] = "audio/x-liquid-file";
    mMetaMap[".lar"] = "application/x-laplayer-reg";
    mMetaMap[".latex"] = "application/x-latex";
    mMetaMap[".lavs"] = "audio/x-liquid-secure";
    mMetaMap[".lbm"] = "application/x-lbm";
    mMetaMap[".lmsff"] = "audio/x-la-lms";
    mMetaMap[".ls"] = "application/x-javascript";
    mMetaMap[".ltr"] = "application/x-ltr";
    mMetaMap[".m1v"] = "video/x-mpeg";
    mMetaMap[".m2v"] = "video/x-mpeg";
    mMetaMap[".m3u"] = "audio/mpegurl";
    mMetaMap[".m4e"] = "video/mpeg4";
    mMetaMap[".mac"] = "application/x-mac";
    mMetaMap[".man"] = "application/x-troff-man";
    mMetaMap[".math"] = "text/xml";
    mMetaMap[".mdb"] = "application/msaccess";
    mMetaMap[".mdb"] = "application/x-mdb";
    mMetaMap[".mfp"] = "application/x-shockwave-flash";
    mMetaMap[".mht"] = "message/rfc822";
    mMetaMap[".mhtml"] = "message/rfc822";
    mMetaMap[".mi"] = "application/x-mi";
    mMetaMap[".mid"] = "audio/mid";
    mMetaMap[".midi"] = "audio/mid";
    mMetaMap[".mil"] = "application/x-mil";
    mMetaMap[".mml"] = "text/xml";
    mMetaMap[".mnd"] = "audio/x-musicnet-download";
    mMetaMap[".mns"] = "audio/x-musicnet-stream";
    mMetaMap[".mocha"] = "application/x-javascript";
    mMetaMap[".movie"] = "video/x-sgi-movie";
    mMetaMap[".mp1"] = "audio/mp1";
    mMetaMap[".mp2"] = "audio/mp2";
    mMetaMap[".mp2v"] = "video/mpeg";
    mMetaMap[".mp3"] = "audio/mp3";
    mMetaMap[".mp4"] = "video/mpeg4";
    mMetaMap[".mpa"] = "video/x-mpg";
    mMetaMap[".mpd"] = "application/vnd.ms-project";
    mMetaMap[".mpe"] = "video/x-mpeg";
    mMetaMap[".mpeg"] = "video/mpg";
    mMetaMap[".mpg"] = "video/mpg";
    mMetaMap[".mpga"] = "audio/rn-mpeg";
    mMetaMap[".mpp"] = "application/vnd.ms-project";
    mMetaMap[".mps"] = "video/x-mpeg";
    mMetaMap[".mpt"] = "application/vnd.ms-project";
    mMetaMap[".mpv"] = "video/mpg";
    mMetaMap[".mpv2"] = "video/mpeg";
    mMetaMap[".mpw"] = "application/vnd.ms-project";
    mMetaMap[".mpx"] = "application/vnd.ms-project";
    mMetaMap[".mtx"] = "text/xml";
    mMetaMap[".mxp"] = "application/x-mmxp";
    mMetaMap[".net"] = "image/pnetvue";
    mMetaMap[".nrf"] = "application/x-nrf";
    mMetaMap[".nws"] = "message/rfc822";
    mMetaMap[".odc"] = "text/x-ms-odc";
    mMetaMap[".out"] = "application/x-out";
    mMetaMap[".p10"] = "application/pkcs10";
    mMetaMap[".p12"] = "application/x-pkcs12";
    mMetaMap[".p7b"] = "application/x-pkcs7-certificates";
    mMetaMap[".p7c"] = "application/pkcs7-mime";
    mMetaMap[".p7m"] = "application/pkcs7-mime";
    mMetaMap[".p7r"] = "application/x-pkcs7-certreqresp";
    mMetaMap[".p7s"] = "application/pkcs7-signature";
    mMetaMap[".pc5"] = "application/x-pc5";
    mMetaMap[".pci"] = "application/x-pci";
    mMetaMap[".pcl"] = "application/x-pcl";
    mMetaMap[".pcx"] = "application/x-pcx";
    mMetaMap[".pdf"] = "application/pdf";
    mMetaMap[".pdf"] = "application/pdf";
    mMetaMap[".pdx"] = "application/vnd.adobe.pdx";
    mMetaMap[".pfx"] = "application/x-pkcs12";
    mMetaMap[".pgl"] = "application/x-pgl";
    mMetaMap[".pic"] = "application/x-pic";
    mMetaMap[".pko"] = "application/vnd.ms-pki.pko";
    mMetaMap[".pl"] = "application/x-perl";
    mMetaMap[".plg"] = "text/html";
    mMetaMap[".pls"] = "audio/scpls";
    mMetaMap[".plt"] = "application/x-plt";
    mMetaMap[".png"] = "image/png";
    // mMetaMap[".png"] = "application/x-png";
    mMetaMap[".pot"] = "application/vnd.ms-powerpoint";
    mMetaMap[".ppa"] = "application/vnd.ms-powerpoint";
    mMetaMap[".ppm"] = "application/x-ppm";
    mMetaMap[".pps"] = "application/vnd.ms-powerpoint";
    mMetaMap[".ppt"] = "application/vnd.ms-powerpoint";
    mMetaMap[".ppt"] = "application/x-ppt";
    mMetaMap[".pr"] = "application/x-pr";
    mMetaMap[".prf"] = "application/pics-rules";
    mMetaMap[".prn"] = "application/x-prn";
    mMetaMap[".prt"] = "application/x-prt";
    mMetaMap[".ps"] = "application/x-ps";
    mMetaMap[".ps"] = "application/postscript";
    mMetaMap[".ptn"] = "application/x-ptn";
    mMetaMap[".pwz"] = "application/vnd.ms-powerpoint";
    mMetaMap[".r3t"] = "text/vnd.rn-realtext3d";
    mMetaMap[".ra"] = "audio/vnd.rn-realaudio";
    mMetaMap[".ram"] = "audio/x-pn-realaudio";
    mMetaMap[".ras"] = "application/x-ras";
    mMetaMap[".rat"] = "application/rat-file";
    mMetaMap[".rdf"] = "text/xml";
    mMetaMap[".rec"] = "application/vnd.rn-recording";
    mMetaMap[".red"] = "application/x-red";
    mMetaMap[".rgb"] = "application/x-rgb";
    mMetaMap[".rjs"] = "application/vnd.rn-realsystem-rjs";
    mMetaMap[".rjt"] = "application/vnd.rn-realsystem-rjt";
    mMetaMap[".rlc"] = "application/x-rlc";
    mMetaMap[".rle"] = "application/x-rle";
    mMetaMap[".rm"] = "application/vnd.rn-realmedia";
    mMetaMap[".rmf"] = "application/vnd.adobe.rmf";
    mMetaMap[".rmi"] = "audio/mid";
    mMetaMap[".rmj"] = "application/vnd.rn-realsystem-rmj";
    mMetaMap[".rmm"] = "audio/x-pn-realaudio";
    mMetaMap[".rmp"] = "application/vnd.rn-rn_music_package";
    mMetaMap[".rms"] = "application/vnd.rn-realmedia-secure";
    mMetaMap[".rmvb"] = "application/vnd.rn-realmedia-vbr";
    mMetaMap[".rmx"] = "application/vnd.rn-realsystem-rmx";
    mMetaMap[".rnx"] = "application/vnd.rn-realplayer";
    mMetaMap[".rp"] = "image/vnd.rn-realpix";
    mMetaMap[".rpm"] = "audio/x-pn-realaudio-plugin";
    mMetaMap[".rsml"] = "application/vnd.rn-rsml";
    mMetaMap[".rt"] = "text/vnd.rn-realtext";
    mMetaMap[".rtf"] = "application/msword";
    mMetaMap[".rtf"] = "application/x-rtf";
    mMetaMap[".rv"] = "video/vnd.rn-realvideo";
    mMetaMap[".sam"] = "application/x-sam";
    mMetaMap[".sat"] = "application/x-sat";
    mMetaMap[".sdp"] = "application/sdp";
    mMetaMap[".sdw"] = "application/x-sdw";
    mMetaMap[".sit"] = "application/x-stuffit";
    mMetaMap[".slb"] = "application/x-slb";
    mMetaMap[".sld"] = "application/x-sld";
    mMetaMap[".slk"] = "drawing/x-slk";
    mMetaMap[".smi"] = "application/smil";
    mMetaMap[".smil"] = "application/smil";
    mMetaMap[".smk"] = "application/x-smk";
    mMetaMap[".snd"] = "audio/basic";
    mMetaMap[".sol"] = "text/plain";
    mMetaMap[".sor"] = "text/plain";
    mMetaMap[".spc"] = "application/x-pkcs7-certificates";
    mMetaMap[".spl"] = "application/futuresplash";
    mMetaMap[".spp"] = "text/xml";
    mMetaMap[".ssm"] = "application/streamingmedia";
    mMetaMap[".sst"] = "application/vnd.ms-pki.certstore";
    mMetaMap[".stl"] = "application/vnd.ms-pki.stl";
    mMetaMap[".stm"] = "text/html";
    mMetaMap[".sty"] = "application/x-sty";
    mMetaMap[".svg"] = "text/xml";
    mMetaMap[".swf"] = "application/x-shockwave-flash";
    mMetaMap[".tdf"] = "application/x-tdf";
    mMetaMap[".tg4"] = "application/x-tg4";
    mMetaMap[".tga"] = "application/x-tga";
    mMetaMap[".tif"] = "image/tiff";
    mMetaMap[".tif"] = "application/x-tif";
    mMetaMap[".tiff"] = "image/tiff";
    mMetaMap[".tld"] = "text/xml";
    mMetaMap[".top"] = "drawing/x-top";
    mMetaMap[".torrent"] = "application/x-bittorrent";
    mMetaMap[".tsd"] = "text/xml";
    mMetaMap[".txt"] = "text/plain";
    mMetaMap[".uin"] = "application/x-icq";
    mMetaMap[".uls"] = "text/iuls";
    mMetaMap[".vcf"] = "text/x-vcard";
    mMetaMap[".vda"] = "application/x-vda";
    mMetaMap[".vdx"] = "application/vnd.visio";
    mMetaMap[".vml"] = "text/xml";
    mMetaMap[".vpg"] = "application/x-vpeg005";
    mMetaMap[".vsd"] = "application/vnd.visio";
    mMetaMap[".vsd"] = "application/x-vsd";
    mMetaMap[".vss"] = "application/vnd.visio";
    mMetaMap[".vst"] = "application/vnd.visio";
    mMetaMap[".vst"] = "application/x-vst";
    mMetaMap[".vsw"] = "application/vnd.visio";
    mMetaMap[".vsx"] = "application/vnd.visio";
    mMetaMap[".vtx"] = "application/vnd.visio";
    mMetaMap[".vxml"] = "text/xml";
    mMetaMap[".wav"] = "audio/wav";
    mMetaMap[".wax"] = "audio/x-ms-wax";
    mMetaMap[".wb1"] = "application/x-wb1";
    mMetaMap[".wb2"] = "application/x-wb2";
    mMetaMap[".wb3"] = "application/x-wb3";
    mMetaMap[".wbmp"] = "image/vnd.wap.wbmp";
    mMetaMap[".wiz"] = "application/msword";
    mMetaMap[".wk3"] = "application/x-wk3";
    mMetaMap[".wk4"] = "application/x-wk4";
    mMetaMap[".wkq"] = "application/x-wkq";
    mMetaMap[".wks"] = "application/x-wks";
    mMetaMap[".wm"] = "video/x-ms-wm";
    mMetaMap[".wma"] = "audio/x-ms-wma";
    mMetaMap[".wmd"] = "application/x-ms-wmd";
    mMetaMap[".wmf"] = "application/x-wmf";
    mMetaMap[".wml"] = "text/vnd.wap.wml";
    mMetaMap[".wmv"] = "video/x-ms-wmv";
    mMetaMap[".wmx"] = "video/x-ms-wmx";
    mMetaMap[".wmz"] = "application/x-ms-wmz";
    mMetaMap[".wp6"] = "application/x-wp6";
    mMetaMap[".wpd"] = "application/x-wpd";
    mMetaMap[".wpg"] = "application/x-wpg";
    mMetaMap[".wpl"] = "application/vnd.ms-wpl";
    mMetaMap[".wq1"] = "application/x-wq1";
    mMetaMap[".wr1"] = "application/x-wr1";
    mMetaMap[".wri"] = "application/x-wri";
    mMetaMap[".wrk"] = "application/x-wrk";
    mMetaMap[".ws"] = "application/x-ws";
    mMetaMap[".ws2"] = "application/x-ws";
    mMetaMap[".wsc"] = "text/scriptlet";
    mMetaMap[".wsdl"] = "text/xml";
    mMetaMap[".wvx"] = "video/x-ms-wvx";
    mMetaMap[".xdp"] = "application/vnd.adobe.xdp";
    mMetaMap[".xdr"] = "text/xml";
    mMetaMap[".xfd"] = "application/vnd.adobe.xfd";
    mMetaMap[".xfdf"] = "application/vnd.adobe.xfdf";
    mMetaMap[".xhtml"] = "text/html";
    mMetaMap[".xls"] = "application/vnd.ms-excel";
    mMetaMap[".xls"] = "application/x-xls";
    mMetaMap[".xlw"] = "application/x-xlw";
    mMetaMap[".xml"] = "text/xml";
    mMetaMap[".xpl"] = "audio/scpls";
    mMetaMap[".xq"] = "text/xml";
    mMetaMap[".xql"] = "text/xml";
    mMetaMap[".xquery"] = "text/xml";
    mMetaMap[".xsd"] = "text/xml";
    mMetaMap[".xsl"] = "text/xml";
    mMetaMap[".xslt"] = "text/xml";
    mMetaMap[".xwd"] = "application/x-xwd";
    mMetaMap[".x_b"] = "application/x-x_b";
    mMetaMap[".sis"] = "application/vnd.symbian.install";
    mMetaMap[".sisx"] = "application/vnd.symbian.install";
    mMetaMap[".x_t"] = "application/x-x_t";
    mMetaMap[".ipa"] = "application/vnd.iphone";
    mMetaMap[".apk"] = "application/vnd.android.package-archive";
    mMetaMap[".xap"] = "application/x-silverlight-app";
}

HttpdConfig* HttpdConfig::instance() {
    if (s_instance == NULL) {
        s_instance = new HttpdConfig();
    }
    return s_instance;
}

std::string HttpdConfig::getMetaType(const std::string& extname)
{
    std::map<std::string, std::string>::iterator it;
    it = mMetaMap.find(extname);
    if (it == mMetaMap.end()) {
        return "text/html";
    }
    return it->second;
}

} // namespace simple_httpd

