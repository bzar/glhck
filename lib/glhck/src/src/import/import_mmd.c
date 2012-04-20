#include "../internal.h"
#include "import.h"
#include <stdio.h>
#include "../../include/mmd.h"

#define GLHCK_CHANNEL GLHCK_CHANNEL_IMPORT
#define ifree(x) if (x) _glhckFree(x);

/* \brief import MikuMikuDance PMD file */
int _glhckImportPMD(_glhckObject *object, const char *file, int animated)
{
   FILE *f;
   size_t i, i2, ix, start, num_faces, num_indices;
   mmd_header header;
   mmd_data *mmd;
   glhckImportVertexData *vertexData = NULL;
   unsigned int *indices = NULL, *strip_indices = NULL;
   CALL("%p, %s, %d", object, file, animated);

   if (!(f = fopen((char*)file, "rb")))
      goto read_fail;

   if (mmd_read_header(f, &header) != 0)
      goto mmd_import_fail;

   if (!(mmd = mmd_new()))
      goto mmd_no_memory;

   if (mmd_read_vertex_data(f, mmd) != 0)
      goto mmd_import_fail;

   if (mmd_read_index_data(f, mmd) != 0)
      goto mmd_import_fail;

   if (mmd_read_material_data(f, mmd) != 0)
      goto mmd_import_fail;

   DEBUG(GLHCK_DBG_CRAP, "V: %d", mmd->num_vertices);
   DEBUG(GLHCK_DBG_CRAP, "I: %d", mmd->num_indices);

   if (!(vertexData = _glhckMalloc(mmd->num_vertices * sizeof(glhckImportVertexData))))
      goto mmd_no_memory;

   if (!(indices = _glhckMalloc(mmd->num_indices * sizeof(unsigned int))))
      goto mmd_no_memory;

   /* init */
   for (i = 0, start = 0; i != mmd->num_materials; ++i) {
      num_faces = mmd->face[i];
      for (i2 = start; i2 != start + num_faces; ++i2) {
         ix = mmd->indices[i2];

         /* vertices */
         vertexData[ix].vertex.x = mmd->vertices[ix * 3 + 0];
         vertexData[ix].vertex.y = mmd->vertices[ix * 3 + 1];
         vertexData[ix].vertex.z = mmd->vertices[ix * 3 + 2];

         /* normals */
         vertexData[ix].normal.x = mmd->normals[ix * 3 + 0];
         vertexData[ix].normal.y = mmd->normals[ix * 3 + 1];
         vertexData[ix].normal.z = mmd->normals[ix * 3 + 2];

         /* texture coords */
         vertexData[ix].coord.x = mmd->coords[ix * 2 + 0];
         vertexData[ix].coord.y = mmd->coords[ix * 2 + 1];

         /* fix coords */
         if (vertexData[ix].coord.x < 0.0f)
            vertexData[ix].coord.x += 1;
         if (vertexData[ix].coord.y < 0.0f)
            vertexData[ix].coord.y += 1;

         indices[i2] = ix;
      }
      start += num_faces;
   }

#if GLHCK_TRISTRIP
   if (!(strip_indices = _glhckTriStrip(indices, mmd->num_indices, &num_indices)))
      goto fail;
   _glhckFree(indices);
#else
   num_indices = mmd->num_indices;
   strip_indices = indices;
#endif

   glhckObjectInsertVertexData(object, mmd->num_vertices, vertexData);
   glhckObjectInsertIndices(object, num_indices, strip_indices);
   _glhckFree(vertexData);
   _glhckFree(strip_indices);

   RET("%d", RETURN_OK);
   return RETURN_OK;

read_fail:
   DEBUG(GLHCK_DBG_ERROR, "Failed to open: %s\n", file);
   goto fail;
mmd_import_fail:
   DEBUG(GLHCK_DBG_ERROR, "MMD importing failed.");
   goto fail;
mmd_no_memory:
   DEBUG(GLHCK_DBG_ERROR, "MMD not enough memory.");
fail:
   if (f) fclose(f);
   ifree(vertexData);
   ifree(indices);
   ifree(strip_indices);
   RET("%d", RETURN_FAIL);
   return RETURN_FAIL;
}