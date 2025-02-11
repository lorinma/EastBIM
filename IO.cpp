/***************************************************************************
# *
# Copyright (c) 2015 *
# Ling Ma <bitly.com/cvlingma> *
# *
# This program is free software; you can redistribute it and/or modify *
# it under the terms of the GNU Lesser General Public License (LGPL) *
# as published by the Free Software Foundation; either version 2 of *
# the License, or (at your option) any later version. *
# for detail see the LICENCE text file. *
# *
# This program is distributed in the hope that it will be useful, *
# but WITHOUT ANY WARRANTY; without even the implied warranty of *
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the *
# GNU Library General Public License for more details. *
# *
# You should have received a copy of the GNU Library General Public *
# License along with this program; if not, write to the Free Software *
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 *
# USA *
# *
# **************************************************************************/

//
// Created by ling on 30/10/15.
//

#include "IO.h"

IO::IO() {

}

void IO::IfcGeometrySetup() {
//    "Specifies whether to apply the local placements of building elements "
//    "directly to the coordinates of the representation mesh rather than "
//    "to represent the local placement in the 4x3 matrix, which will in that "
//    "case be the identity matrix."
    IfcGeomObjects::Settings(IfcGeomObjects::USE_WORLD_COORDS, true);
//    "Specifies whether vertices are welded, meaning that the coordinates "
//    "vector will only contain unique xyz-triplets. This results in a "
//    "manifold mesh which is useful for modelling applications, but might "
//    "result in unwanted shading artefacts in rendering applications."
    IfcGeomObjects::Settings(IfcGeomObjects::WELD_VERTICES, true);
//    Specifies whether to sew IfcConnectedFaceSets (open and closed shells) to
//    TopoDS_Shells or whether to keep them as a loose collection of faces.
    IfcGeomObjects::Settings(IfcGeomObjects::SEW_SHELLS, false);
//     Internally IfcOpenShell measures everything in meters. This settings
//     specifies whether to convert IfcGeomObjects back to the units in which
//     the geometry in the IFC file is specified.
    IfcGeomObjects::Settings(IfcGeomObjects::CONVERT_BACK_UNITS, true);
//     Specifies whether to compose IfcOpeningElements into a single compound
//     in order to speed up the processing of opening subtractions.
    IfcGeomObjects::Settings(IfcGeomObjects::FASTER_BOOLEANS, true);
    IfcGeomObjects::Settings(IfcGeomObjects::FORCE_CCW_FACE_ORIENTATION, true);
//    "Specifies whether to disable the boolean subtraction of "
//    "IfcOpeningElement Representations from their RelatingElements."
    IfcGeomObjects::Settings(IfcGeomObjects::DISABLE_OPENING_SUBTRACTIONS, false);
}

bool IO::LoadIfcModel(string filename, BIM::Ptr model) {
    IfcGeometrySetup();
    static std::stringstream log_stream;
    // Parse the file supplied in argv[1]. Returns true on succes.
    if ( ! IfcGeomObjects::Init(filename, &std::cout, &log_stream) ) {
        Logger::Message(Logger::LOG_ERROR, "Unable to parse .ifc file or no geometrical entities found");
        return false;
    }

    // The functions IfcGeomObjects::Get() and IfcGeomObjects::Next() wrap an iterator of all geometrical entities in the Ifc file.
    // IfcGeomObjects::Get() returns an IfcGeomObjects::IfcGeomObject (see IfcGeomObjects.h for definition)
    // IfcGeomObjects::Next() is used to poll whether more geometrical entities are available
    do {
        const IfcGeomObjects::IfcObject* geom_object = IfcGeomObjects::GetShapeModel();
//        int id=geom_object->id();
//        string guid = geom_object->guid();
//        string name = geom_object->name();
//        string type = geom_object->type();
        model->elements.push_back(new BldElement(geom_object->id(),geom_object->guid(),geom_object->name(),geom_object->type(),static_cast<const IfcGeomObjects::IfcGeomShapeModelObject*>(geom_object)));
    } while (IfcGeomObjects::Next());
    return true;
}

bool IO::GetParas(boost::ptr_vector<string> &list, const int &sheetInx, const int &column) {
    string filename="parameters.xls";
    ExcelFormat::BasicExcel xls;
    if(!xls.Load(filename.c_str()))
        return false;
    assert(xls.GetTotalWorkSheets()>sheetInx);
    ExcelFormat::BasicExcelWorksheet *sheet=xls.GetWorksheet(sheetInx);
    int paraCount=sheet->GetTotalRows();
    if(paraCount<1)
        return false;
    bool hasValue=false;
    //skip the header
    for (int i=1;i<paraCount;i++){
        const char* value=sheet->Cell(i,column)->GetString();
        if(value==NULL)//only return continuous content
            break;
        list.push_back(new string(value));
        hasValue=true;
    }
    return hasValue;
}

bool IO::GetIfcFiles(boost::ptr_vector<string> &list) {
    return GetParas(list,0,0);
}
