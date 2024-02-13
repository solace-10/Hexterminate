# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8-80 compliant>

import array
import os
import time
import bpy
import mathutils
import struct
import bmesh
from bpy_extras.image_utils import load_image


###############################################################################
# TMF exporter
###############################################################################

def getVersion():
    return 91


def getHelperCount():
    count = 0

    for element in bpy.context.scene.objects:
        if element.type == "EMPTY":
            count = count + 1

    return count


def getObjectCount():
    count = 0
    
    for element in bpy.context.scene.objects:
        if element.type == "MESH":
            count = count + 1
    
    return count


def getUVCount(bm):
    count = 0

    for face in bm.faces:
        count = count + len(face.loops)
    
    return count


current_material_index = 1
materials = {}


def getMaterialIndex(object):
    global current_material_index
    global materials
    
    for slot in object.material_slots:
        if slot.material is not None:
            
            if slot.material.name in materials.values():
                return materials[slot.material.name]
            else:
                materials[slot.material.name] = current_material_index
                print("Material", slot.material.name, ":", current_material_index)
                current_material_index = current_material_index + 1
                return current_material_index - 1
        else:
            return 0
    return 0


def writeObjectHeader(file, object, bm):
    material_index = getMaterialIndex(object)
    num_vertices = len(bm.verts)
    num_uvs = getUVCount(bm)
    num_triangles = len(bm.faces)

    print("Header:")
    print("- Material index:", material_index)
    print("- Vertices:", num_vertices)
    print("- UVs:", num_uvs)
    print("- Triangles:", num_triangles)

    header = struct.pack("=IIII", material_index, num_vertices, num_uvs, num_triangles)
    file.write(header)


def writeVertices(file, bm):
    print("Vertex data:")
    index = 0
    for vert in bm.verts:
        data = struct.pack("=fff", vert.co.x, vert.co.y, vert.co.z)
        file.write(data)
        print(index, ":", vert.co.x, vert.co.y, vert.co.z)
        index = index + 1
        
        
def writeUVs(file, bm):
    uv_lay = bm.loops.layers.uv.active
    for face in bm.faces:
        for loop in face.loops:
            uv = loop[uv_lay].uv
            data = struct.pack("=ff", uv.x, uv.y)
            file.write(data)
            
            
def writeTriangles(file, bm):
    uvIndex = 0
    for face in bm.faces:
        vertexIndices = struct.pack("=III", face.loops[0].vert.index, face.loops[1].vert.index, face.loops[2].vert.index)
        uvs = struct.pack("=III", uvIndex, uvIndex + 1, uvIndex + 2)
        normals = struct.pack("=fffffffff",
            face.loops[0].vert.normal.x, face.loops[0].vert.normal.y, face.loops[0].vert.normal.z,
            face.loops[1].vert.normal.x, face.loops[1].vert.normal.y, face.loops[1].vert.normal.z,
            face.loops[2].vert.normal.x, face.loops[2].vert.normal.y, face.loops[2].vert.normal.z)
        uvIndex = uvIndex + 3
        file.write(vertexIndices)
        file.write(uvs)
        file.write(normals)


def writeHelper(file, helper):
    print("=== Writing helper", helper.name, "===")
    nameLength = struct.pack("=I", len(helper.name))
    name = bytes(helper.name, "ascii")
    location = struct.pack("=fff", helper.location.x, helper.location.y, helper.location.z)
    file.write(nameLength + name + location)


def writeObject(file, object_id, object):
    print("=== Writing object", object.name, "===")
    print("ID:", object_id)
    
    # Triangulate the mesh
    bm = bmesh.new()
    bm.from_object(object, depsgraph=bpy.context.evaluated_depsgraph_get())
    bmesh.ops.triangulate(bm, faces=bm.faces[:], quad_method="BEAUTY", ngon_method="BEAUTY")
    
    writeObjectHeader(file, object, bm)
    writeVertices(file, bm)
    writeUVs(file, bm)
    writeTriangles(file, bm)


def writeHeader(file):
    header = struct.pack("=cccHHH", b"T", b"M", b"F", getVersion(), getObjectCount(), getHelperCount())
    file.write(header)


def writeHelpers(file):
    for element in bpy.context.scene.objects:
        if element.type == "EMPTY":
            writeHelper(file, element)


def writeObjects(file):
    id = 0
    for element in bpy.context.scene.objects:
        if element.type == "MESH":
              writeObject(file, id, element)
              id = id + 1          


def saveTMF(context,
         filepath,
         *,
         global_matrix=None,
         path_mode="AUTO"
         ):
       
    global current_material_index
    global materials

    current_material_index = 1
    materials.clear()

    with open(filepath, "wb") as file:
        writeHeader(file)
        writeHelpers(file)
        writeObjects(file)
        file.close()

    print("Export finished.")

    return {"FINISHED"}


###############################################################################
# TMF importer
###############################################################################

loadedMaterials = []


def createMeshFromData(name, origin, triangles, vertices, uvs):
    # Create mesh and object
    me = bpy.data.meshes.new(name+'Mesh')
    ob = bpy.data.objects.new(name, me)
    ob.location = origin
    ob.show_name = False
 
    bpy.context.collection.objects.link(ob)
 
    # Create a list of faces from the triangle's vertex indices
    faces = []
    for triangle in triangles:
        faces.append(triangle.vertex_indices)
   
    # Create mesh from given vertices and faces.
    me.from_pydata(vertices, [], faces)

    # Create UVs
    me.uv_layers.new(do_init=False)
    uvl = me.uv_layers[0].data
    for f in me.polygons:
        print("Polygon", f.index, "from loop index", f.loop_start, "and length", f.loop_total)
        for k,i in enumerate(f.loop_indices): # Python Range object with the proper indices already set
            l = me.loops[i] # The loop entry this polygon point refers to
            v = me.vertices[l.vertex_index] # The vertex data that loop entry refers to
            print("\tLoop index", l.index, "points to vertex index", l.vertex_index)
            for j,ul in enumerate(me.uv_layers):
                u = uvs[triangles[f.index].uv_indices[k]].u
                v = uvs[triangles[f.index].uv_indices[k]].v
                uvl[l.index].uv = (u, v)
                print("\t\tUV Map", j, "has coordinates", uvl[l.index].uv, "for this loop index")

    # Update mesh with new data
    me.update() 

    return ob
    
    
def loadHelper(file):
    print("loadHelper() not implemented yet")
    name_length = int.from_bytes(file.read(4), byteorder="little")
    self.name = file.read(name_length).decode("ascii")
    self.position = mathutils.Vector(struct.unpack("fff", file.read(12)))

    
class UV:
    def __init__(self, packed_structure):
        self.u = packed_structure[0]
        self.v = packed_structure[1]
        
        
class Triangle:
    def __init__(self, packed_structure):
        self.vertex_indices = packed_structure[0:3]
        self.uv_indices = packed_structure[3:6]
        self.normals = []
        self.normals.append(mathutils.Vector(packed_structure[6:9]))
        self.normals.append(mathutils.Vector(packed_structure[9:12]))
        self.normals.append(mathutils.Vector(packed_structure[12:15]))
       
       
def loadObject(file, object_index):
    print("Loading object", object_index, "...")

    material_index = int.from_bytes(file.read(4), byteorder="little") - 1
    vertex_count = int.from_bytes(file.read(4), byteorder="little")
    uv_count = int.from_bytes(file.read(4), byteorder="little")
    triangle_count = int.from_bytes(file.read(4), byteorder="little")
    
    print("- Material index:", material_index)
    print("- Vertex count:", vertex_count)
    print("- UV count:", uv_count)
    print("- Triangle count:", triangle_count)
    
    vertices = []
    for vertex in range(0, vertex_count):
        vertices.append(mathutils.Vector(struct.unpack("fff", file.read(12))))
       
    uvs = []
    for uv in range(0, uv_count):
        uvs.append(UV(struct.unpack("ff", file.read(8))))
        
    triangles = []
    for triangle_index in range(0, triangle_count):
        triangles.append(Triangle(struct.unpack("iiiiiifffffffff", file.read(60))))

    # Create geometry
    origin = mathutils.Vector((0,0,0))
    ob = createMeshFromData("Object{0}".format(object_index + 1), origin, triangles, vertices, uvs)
    
    # Assign the previously loaded material to the geometry
    #if ob.data.materials:
    #    ob.data.materials[0] = loadedMaterials[material_index]
    #else:
    #    ob.data.materials.append(loadedMaterials[material_index])
    
    
def loadMaterials(filepath):
    materialFilepath = os.path.splitext(filepath)[0]+".tml"
    with open(materialFilepath, "r") as file:
        texture_counter = 0
        for line in file:
            if line.startswith("  ") == False:
                material_name = line[:-1]
                
                # If a material with this name already exists, remove it and create a new one.
                current_material = bpy.data.materials.get(material_name)
                if current_material is not None:
                    bpy.data.materials.remove(current_material, do_unlink=True)
                    
                current_material = bpy.data.materials.new(name=material_name)
               
                loadedMaterials.append(current_material)
            elif line.startswith("  TEXTUREMAP"):
                texture_name = "Texture{0}".format(texture_counter)
                texture_counter += 1
                texture = bpy.data.textures.new(name=texture_name, type="IMAGE")

                textureFilepath = os.path.dirname(filepath) + os.sep + line[13:]
                print(textureFilepath)
                texture.image = load_image(line[13:-1], os.path.dirname(filepath), place_holder=True)
                if texture.image is None:
                    print("Couldn't load texture " + textureFilepath)

                mtex = current_material.texture_slots.add()
                mtex.texture = texture
                mtex.texture_coords = "UV"

                # Assumes the first texture loaded is the diffuse and that the second is the specular. 
                # This is true for the majority of the materials and makes importing easier.
                num_texture_slots = len(current_material.texture_slots)
                if num_texture_slots == 1:
                    mtex.use_map_color_diffuse = True
                    mtex.use_map_diffuse = True
                
    
def loadTMF(context,
         filepath,
         *,
         global_clamp_size=0.0,
         use_smooth_groups=True,
         use_edges=True,
         use_split_objects=True,
         use_split_groups=True,
         use_image_search=True,
         use_groups_as_vgroups=False,
         relpath=None,
         global_matrix=None
         ):

    #loadMaterials(filepath)

    with open(filepath, "rb") as file:
        format = file.read(3)
        
        #print("format: {0}".format(format))
        version = int.from_bytes(file.read(2), byteorder="little")
        print("version: {0}".format(version))
        object_count = int.from_bytes(file.read(2), byteorder="little")
        print("geometry objects: {0}".format(object_count))
        helper_count = int.from_bytes(file.read(2), byteorder="little")
        print("helper objects: {0}".format(helper_count))
        
        for helper_index in range(0, helper_count):
            loadHelper(file)
            
        for object_index in range(0, object_count):
            loadObject(file, object_index)

    return {'FINISHED'}



###############################################################################
# Addon information and import/export dialogs.
###############################################################################

bl_info = {
    "name": "Genesis TMF format",
    "author": "Pedro Nunes",
    "version": (1, 0, 0),
    "blender": (2, 83, 0),
    "location": "File > Import-Export",
    "description": "Import-Export TMF",
    "warning": "",
    "wiki_url": "",
    "support": 'TESTING',
    "category": "Import-Export",
}

import bpy
from bpy.props import (
        BoolProperty,
        FloatProperty,
        StringProperty,
        EnumProperty,
        )
from bpy_extras.io_utils import (
        ImportHelper,
        ExportHelper,
        orientation_helper,
        path_reference_mode,
        axis_conversion,
        )


@orientation_helper(axis_forward='-Z', axis_up='Y')
class ImportTMF(bpy.types.Operator, ImportHelper):
    """Load a Genesis TMF File"""
    bl_idname = "import_scene.tmf"
    bl_label = "Import TMF"
    bl_options = {'PRESET', 'UNDO'}

    filename_ext = ".tmf"
    filter_glob: StringProperty(
            default="*.tmf;*.mtl",
            options={'HIDDEN'},
            )

    use_edges = True
    use_smooth_groups = True
    use_split_objects = False
    use_split_groups = False
    use_groups_as_vgroups = False
    use_image_search = True

    split_mode: EnumProperty(
            name="Split",
            items=(('ON', "Split", "Split geometry, omits unused verts"),
                   ('OFF', "Keep Vert Order", "Keep vertex order from file"),
                   ),
            )

    def execute(self, context):
        # print("Selected: " + context.active_object.name)

        if self.split_mode == 'OFF':
            self.use_split_objects = False
            self.use_split_groups = False
        else:
            self.use_groups_as_vgroups = False

        keywords = self.as_keywords(ignore=("axis_forward",
                                            "axis_up",
                                            "filter_glob",
                                            "split_mode",
                                            ))

        global_matrix = axis_conversion(from_forward=self.axis_forward,
                                        from_up=self.axis_up,
                                        ).to_4x4()
        keywords["global_matrix"] = global_matrix

        if bpy.data.is_saved and context.preferences.filepaths.use_relative_paths:
            import os
            keywords["relpath"] = os.path.dirname(bpy.data.filepath)

        return loadTMF(context, **keywords)

    def draw(self, context):
        pass
        

class TMF_PT_import_geometry(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Geometry"
    bl_parent_id = "FILE_PT_operator"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "IMPORT_SCENE_OT_tmf"

    def draw(self, context):
        layout = self.layout

        sfile = context.space_data
        operator = sfile.active_operator

        layout.row().prop(operator, "split_mode", expand=True)

        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        col = layout.column()
        if operator.split_mode == 'ON':
            col.prop(operator, "use_split_objects", text="Split by Object")
            col.prop(operator, "use_split_groups", text="Split by Group")
        else:
            col.prop(operator, "use_groups_as_vgroups")


class TMF_PT_export_include(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Include"
    bl_parent_id = "FILE_PT_operator"

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_tmf"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator


class TMF_PT_export_transform(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Transform"
    bl_parent_id = "FILE_PT_operator"

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_tmf"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator

        layout.prop(operator, 'path_mode')
        layout.prop(operator, 'axis_forward')
        layout.prop(operator, 'axis_up')


class TMF_PT_export_geometry(bpy.types.Panel):
    bl_space_type = 'FILE_BROWSER'
    bl_region_type = 'TOOL_PROPS'
    bl_label = "Geometry"
    bl_parent_id = "FILE_PT_operator"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        sfile = context.space_data
        operator = sfile.active_operator

        return operator.bl_idname == "EXPORT_SCENE_OT_tmf"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False  # No animation.

        sfile = context.space_data
        operator = sfile.active_operator

        layout.prop(operator, 'use_mesh_modifiers')
        # Property definition disabled, not working in 2.8 currently.
        # layout.prop(operator, 'use_mesh_modifiers_render')
        layout.prop(operator, 'use_smooth_groups')
        layout.prop(operator, 'use_smooth_groups_bitflags')
        layout.prop(operator, 'use_normals')
        layout.prop(operator, 'use_uvs')
        layout.prop(operator, 'use_materials')
        layout.prop(operator, 'use_triangles')
        layout.prop(operator, 'use_nurbs', text="Curves as NURBS")
        layout.prop(operator, 'use_vertex_groups')
        layout.prop(operator, 'keep_vertex_order')


@orientation_helper(axis_forward='-Z', axis_up='Y')
class ExportTMF(bpy.types.Operator, ExportHelper):
    """Save a Genesis TMF File"""

    bl_idname = "export_scene.tmf"
    bl_label = 'Export TMF'
    bl_options = {'PRESET'}

    filename_ext = ".tmf"
    filter_glob: StringProperty(
            default="*.tmf;*.mtl",
            options={'HIDDEN'},
            )

    path_mode: path_reference_mode

    check_extension = True

    def execute(self, context):
        from mathutils import Matrix
        keywords = self.as_keywords(ignore=("axis_forward",
                                            "axis_up",
                                            "check_existing",
                                            "filter_glob",
                                            ))

        global_matrix = (Matrix.Scale(1, 4) @
                         axis_conversion(to_forward=self.axis_forward,
                                         to_up=self.axis_up,
                                         ).to_4x4())

        keywords["global_matrix"] = global_matrix
        return saveTMF(context, **keywords)

    def draw(self, context):
        pass


def menu_func_import(self, context):
    self.layout.operator(ImportTMF.bl_idname, text="Genesis (.tmf)")


def menu_func_export(self, context):
    self.layout.operator(ExportTMF.bl_idname, text="Genesis (.tmf)")


classes = (
    ImportTMF,
    TMF_PT_import_geometry,
    ExportTMF,
    TMF_PT_export_include,
    TMF_PT_export_transform,
    TMF_PT_export_geometry,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)

    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)

    for cls in classes:
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
