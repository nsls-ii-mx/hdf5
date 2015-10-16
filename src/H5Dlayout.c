/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/****************/
/* Module Setup */
/****************/

#define H5D_PACKAGE		/*suppress error about including H5Dpkg	  */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Dpkg.h"		/* Datasets 				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5HLprivate.h"	/* Local heaps				*/


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:	H5D__layout_set_io_ops
 *
 * Purpose:	Set the I/O operation function pointers for a dataset,
 *              according to the dataset's layout
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Thursday, March 20, 2008
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D__layout_set_io_ops(const H5D_t *dataset)
{
    herr_t ret_value = SUCCEED;		/* Return value */

    FUNC_ENTER_PACKAGE

    /* check args */
    HDassert(dataset);

    /* Set the I/O functions for each layout type */
    switch(dataset->shared->layout.type) {
        case H5D_CONTIGUOUS:
            if(dataset->shared->dcpl_cache.efl.nused > 0)
                dataset->shared->layout.ops = H5D_LOPS_EFL;
            else
                dataset->shared->layout.ops = H5D_LOPS_CONTIG;
            break;

        case H5D_CHUNKED:
            dataset->shared->layout.ops = H5D_LOPS_CHUNK;

            /* Set the chunk operations */
            switch(dataset->shared->layout.u.chunk.idx_type) {
                case H5D_CHUNK_IDX_BTREE:
                    dataset->shared->layout.storage.u.chunk.ops = H5D_COPS_BTREE;
                    break;

                case H5D_CHUNK_IDX_NONE:
                    dataset->shared->layout.storage.u.chunk.ops = H5D_COPS_NONE;
                    break;

                case H5D_CHUNK_IDX_FARRAY:
                    dataset->shared->layout.storage.u.chunk.ops = H5D_COPS_FARRAY;
                    break;

                case H5D_CHUNK_IDX_EARRAY:
                    dataset->shared->layout.storage.u.chunk.ops = H5D_COPS_EARRAY;
                    break;

                case H5D_CHUNK_IDX_BT2:
                    dataset->shared->layout.storage.u.chunk.ops = H5D_COPS_BT2;
                    break;

                default:
                    HDassert(0 && "Unknown chunk index method!");
                    HGOTO_ERROR(H5E_DATASET, H5E_UNSUPPORTED, FAIL, "unknown chunk index method")
            } /* end switch */
            break;

        case H5D_COMPACT:
            dataset->shared->layout.ops = H5D_LOPS_COMPACT;
            break;

        case H5D_LAYOUT_ERROR:
        case H5D_NLAYOUTS:
        default:
            HGOTO_ERROR(H5E_DATASET, H5E_UNSUPPORTED, FAIL, "unknown storage method")
    } /* end switch */ /*lint !e788 All appropriate cases are covered */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D__layout_set_io_ops() */


/*-------------------------------------------------------------------------
 * Function:    H5D__layout_meta_size
 *
 * Purpose:     Returns the size of the raw message in bytes except raw data
 *              part for compact dataset.  This function doesn't take into
 *              account message alignment.
 *
 * Return:      Success:        Message data size in bytes
 *              Failure:        0
 *
 * Programmer:  Raymond Lu
 *              August 14, 2002
 *
 *-------------------------------------------------------------------------
 */
size_t
H5D__layout_meta_size(const H5F_t *f, const H5O_layout_t *layout, hbool_t include_compact_data)
{
    size_t                  ret_value;

    FUNC_ENTER_PACKAGE

    /* check args */
    HDassert(f);
    HDassert(layout);

    ret_value = 1 +                     /* Version number                       */
                1;                      /* layout class type                    */

    switch(layout->type) {
        case H5D_COMPACT:
            /* This information only present in older versions of message */
            /* Size of raw data */
            ret_value += 2;
            if(include_compact_data)
                ret_value += layout->storage.u.compact.size; /* data for compact dataset             */
            break;

        case H5D_CONTIGUOUS:
            /* This information only present in older versions of message */
            ret_value += H5F_SIZEOF_ADDR(f);    /* Address of data */
            ret_value += H5F_SIZEOF_SIZE(f);    /* Length of data */
            break;

        case H5D_CHUNKED:
            if(layout->version < H5O_LAYOUT_VERSION_4) {
                /* Number of dimensions (1 byte) */
                HDassert(layout->u.chunk.ndims > 0 && layout->u.chunk.ndims <= H5O_LAYOUT_NDIMS);
                ret_value++;

                /* B-tree address */
                ret_value += H5F_SIZEOF_ADDR(f);    /* Address of data */

                /* Dimension sizes */
                ret_value += layout->u.chunk.ndims * 4;
            } /* end if */
            else {
                /* Chunked layout feature flags */
                ret_value++;

                /* Number of dimensions (1 byte) */
                HDassert(layout->u.chunk.ndims > 0 && layout->u.chunk.ndims <= H5O_LAYOUT_NDIMS);
                ret_value++;

                /* Encoded # of bytes for each chunk dimension */
                HDassert(layout->u.chunk.enc_bytes_per_dim > 0 && layout->u.chunk.enc_bytes_per_dim <= 8);
                ret_value++;

                /* Dimension sizes */
                ret_value += layout->u.chunk.ndims * layout->u.chunk.enc_bytes_per_dim;

                /* Type of chunk index */
                ret_value++;

                switch(layout->u.chunk.idx_type) {
                    case H5D_CHUNK_IDX_NONE:
			/* nothing */
			break;

                    case H5D_CHUNK_IDX_FARRAY:
                        /* Fixed array creation parameters */
                        ret_value += H5D_FARRAY_CREATE_PARAM_SIZE;
                        break;

                    case H5D_CHUNK_IDX_EARRAY:
                        /* Extensible array creation parameters */
                        ret_value += H5D_EARRAY_CREATE_PARAM_SIZE;
                        break;

		    case H5D_CHUNK_IDX_BT2:
                        /* v2 B-tree creation parameters */
                        ret_value += H5D_BT2_CREATE_PARAM_SIZE;
                        break;

                    default:
                        HGOTO_ERROR(H5E_OHDR, H5E_CANTENCODE, 0, "Invalid chunk index type")
                } /* end switch */

                /* Chunk index address */
                ret_value += H5F_SIZEOF_ADDR(f);
            } /* end else */
            break;

        case H5D_LAYOUT_ERROR:
        case H5D_NLAYOUTS:
        default:
            HGOTO_ERROR(H5E_OHDR, H5E_CANTENCODE, 0, "Invalid layout class")
    } /* end switch */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D__layout_meta_size() */


/*-------------------------------------------------------------------------
 * Function:    H5D__layout_set_latest_version
 *
 * Purpose:     Set the encoding for a layout to the latest version.
 *		Part of the coding in this routine is moved to
 *		H5D__layout_set_latest_indexing().
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              Thursday, January 15, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D__layout_set_latest_version(H5O_layout_t *layout, const H5S_t *space, 
    const H5D_dcpl_cache_t *dcpl_cache)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_PACKAGE

    /* Sanity check */
    HDassert(layout);
    HDassert(space);
    HDassert(dcpl_cache);

    /* Set encoding of layout to latest version */
    layout->version = H5O_LAYOUT_VERSION_LATEST;

    /* Set the latest indexing type for the layout message */
    if(H5D__layout_set_latest_indexing(layout, space, dcpl_cache) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set latest indexing type")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D__layout_set_latest_version() */


/*-------------------------------------------------------------------------
 * Function:    H5D__layout_set_latest_indexing
 *
 * Purpose:     Set the latest indexing type for a layout message
 *		This is moved from H5D_layout_set_latest_version().
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              Thursday, January 15, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D__layout_set_latest_indexing(H5O_layout_t *layout, const H5S_t *space, 
    const H5D_dcpl_cache_t *dcpl_cache)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_PACKAGE

    /* Sanity check */
    HDassert(layout);
    HDassert(space);
    HDassert(dcpl_cache);

    /* The indexing methods only apply to chunked datasets (currently) */
    if(layout->type == H5D_CHUNKED) {
        int sndims;                         /* Rank of dataspace */
        unsigned ndims;                     /* Rank of dataspace */

        /* Query the dimensionality of the dataspace */
        if((sndims = H5S_GET_EXTENT_NDIMS(space)) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "invalid dataspace rank")
        ndims = (unsigned)sndims;

        /* Avoid scalar/null dataspace */
        if(ndims > 0) {
            hsize_t max_dims[H5O_LAYOUT_NDIMS];     /* Maximum dimension sizes */
            unsigned unlim_count = 0;           	/* Count of unlimited max. dimensions */
            unsigned u;                     /* Local index variable */

            /* Query the dataspace's dimensions */
            if(H5S_get_simple_extent_dims(space, NULL, max_dims) < 0)
                HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't get dataspace max. dimensions")

            /* Spin through the max. dimensions, looking for unlimited dimensions */
            for(u = 0; u < ndims; u++)
                if(max_dims[u] == H5S_UNLIMITED)
                    unlim_count++;

            /* Chunked datasets with unlimited dimension(s) */
            if(unlim_count) { /* dataset with unlimited dimension(s) must be chunked */
                if(1 == unlim_count) { /* Chunked dataset with only 1 unlimited dimension */
                    /* Set the chunk index type to an extensible array */
                    layout->u.chunk.idx_type = H5D_CHUNK_IDX_EARRAY;
                    layout->storage.u.chunk.idx_type = H5D_CHUNK_IDX_EARRAY;
                    layout->storage.u.chunk.ops = H5D_COPS_EARRAY;

                    /* Set the extensible array creation parameters */
                    /* (use hard-coded defaults for now, until we give applications
                     *          control over this with a property list - QAK)
                     */
                    layout->u.chunk.u.earray.cparam.max_nelmts_bits = H5D_EARRAY_MAX_NELMTS_BITS;
                    layout->u.chunk.u.earray.cparam.idx_blk_elmts = H5D_EARRAY_IDX_BLK_ELMTS;
                    layout->u.chunk.u.earray.cparam.sup_blk_min_data_ptrs = H5D_EARRAY_SUP_BLK_MIN_DATA_PTRS;
                    layout->u.chunk.u.earray.cparam.data_blk_min_elmts = H5D_EARRAY_DATA_BLK_MIN_ELMTS;
                    layout->u.chunk.u.earray.cparam.max_dblk_page_nelmts_bits = H5D_EARRAY_MAX_DBLOCK_PAGE_NELMTS_BITS;
                } /* end if */
                else { /* Chunked dataset with > 1 unlimited dimensions */
                    /* Set the chunk index type to v2 B-tree */
                    layout->u.chunk.idx_type = H5D_CHUNK_IDX_BT2;
                    layout->storage.u.chunk.idx_type = H5D_CHUNK_IDX_BT2;
                    layout->storage.u.chunk.ops = H5D_COPS_BT2;

                    /* Set the v2 B-tree creation parameters */
                    layout->u.chunk.u.btree2.cparam.node_size = H5D_BT2_NODE_SIZE;
                    layout->u.chunk.u.btree2.cparam.split_percent = H5D_BT2_SPLIT_PERC;
                    layout->u.chunk.u.btree2.cparam.merge_percent =  H5D_BT2_MERGE_PERC;
                } /* end else */
            } /* end if */
            else {      /* Chunked dataset with fixed dimensions */
                /* Check for correct condition for using "implicit" chunk index */
                if(!dcpl_cache->pline.nused && 
                        dcpl_cache->fill.alloc_time == H5D_ALLOC_TIME_EARLY) {

                    /* Set the chunk index type to Non Index */
                    layout->u.chunk.idx_type = H5D_CHUNK_IDX_NONE;
                    layout->storage.u.chunk.idx_type = H5D_CHUNK_IDX_NONE;
                    layout->storage.u.chunk.ops = H5D_COPS_NONE;
                } /* end if */
                else { /* Used Fixed Array */
                    /* Set the chunk index type to Fixed Array */
                    layout->u.chunk.idx_type = H5D_CHUNK_IDX_FARRAY;
                    layout->storage.u.chunk.idx_type = H5D_CHUNK_IDX_FARRAY;
                    layout->storage.u.chunk.ops = H5D_COPS_FARRAY;

                    /* Set the fixed array creation parameters */
                    /* (use hard-coded defaults for now, until we give applications
                     *          control over this with a property list - QAK)
                     */
                    layout->u.chunk.u.farray.cparam.max_dblk_page_nelmts_bits = H5D_FARRAY_MAX_DBLK_PAGE_NELMTS_BITS;
                } /* end else */
            } /* end else */
        } /* end if */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D__layout_set_latest_indexing() */


/*-------------------------------------------------------------------------
 * Function:	H5D__layout_oh_create
 *
 * Purpose:	Create layout/pline/efl information for dataset
 *
 * Return:	Success:    SUCCEED
 *		Failure:    FAIL
 *
 * Programmer:	Quincey Koziol
 *		Monday, July 27, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D__layout_oh_create(H5F_t *file, hid_t dxpl_id, H5O_t *oh, H5D_t *dset,
    hid_t dapl_id)
{
    H5O_layout_t        *layout;         /* Dataset's layout information */
    const H5O_fill_t	*fill_prop;     /* Pointer to dataset's fill value information */
    hbool_t             layout_init = FALSE;    /* Flag to indicate that chunk information was initialized */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_PACKAGE_TAG(dxpl_id, dset->oloc.addr, FAIL)

    /* Sanity checking */
    HDassert(file);
    HDassert(oh);
    HDassert(dset);

    /* Set some local variables, for convenience */
    layout = &dset->shared->layout;
    fill_prop = &dset->shared->dcpl_cache.fill;

    /* Update the filters message, if this is a chunked dataset */
    if(layout->type == H5D_CHUNKED) {
        H5O_pline_t     *pline;         /* Dataset's I/O pipeline information */

        pline = &dset->shared->dcpl_cache.pline;
        if(pline->nused > 0 && H5O_msg_append_oh(file, dxpl_id, oh, H5O_PLINE_ID, H5O_MSG_FLAG_CONSTANT, 0, pline) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to update filter header message")
    } /* end if */

    /* Initialize the layout information for the new dataset */
    if(dset->shared->layout.ops->init && (dset->shared->layout.ops->init)(file, dxpl_id, dset, dapl_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to initialize layout information")

    /* Indicate that the layout information was initialized */
    layout_init = TRUE;

    /*
     * Allocate storage if space allocate time is early; otherwise delay
     * allocation until later.
     */
    if(fill_prop->alloc_time == H5D_ALLOC_TIME_EARLY)
        if(H5D__alloc_storage(dset, dxpl_id, H5D_ALLOC_CREATE, FALSE, NULL) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to initialize storage")

    /* Update external storage message, if it's used */
    if(dset->shared->dcpl_cache.efl.nused > 0) {
        H5O_efl_t *efl = &dset->shared->dcpl_cache.efl; /* Dataset's external file list */
        H5HL_t *heap;                           /* Pointer to local heap for EFL file names */
        size_t heap_size = H5HL_ALIGN(1);
        size_t u;

        /* Determine size of heap needed to stored the file names */
        for(u = 0; u < efl->nused; ++u)
            heap_size += H5HL_ALIGN(HDstrlen(efl->slot[u].name) + 1);

        /* Create the heap for the EFL file names */
        if(H5HL_create(file, dxpl_id, heap_size, &efl->heap_addr/*out*/) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to create EFL file name heap")

        /* Pin the heap down in memory */
        if(NULL == (heap = H5HL_protect(file, dxpl_id, efl->heap_addr, H5AC__NO_FLAGS_SET)))
            HGOTO_ERROR(H5E_DATASET, H5E_CANTPROTECT, FAIL, "unable to protect EFL file name heap")

        /* Insert "empty" name first */
        if(UFAIL == H5HL_insert(file, dxpl_id, heap, (size_t)1, "")) {
            H5HL_unprotect(heap);
            HGOTO_ERROR(H5E_DATASET, H5E_CANTINSERT, FAIL, "unable to insert file name into heap")
        } /* end if */

        for(u = 0; u < efl->nused; ++u) {
            size_t offset;      /* Offset of file name in heap */

            /* Insert file name into heap */
            if(UFAIL == (offset = H5HL_insert(file, dxpl_id, heap,
                        HDstrlen(efl->slot[u].name) + 1, efl->slot[u].name))) {
                H5HL_unprotect(heap);
                HGOTO_ERROR(H5E_DATASET, H5E_CANTINSERT, FAIL, "unable to insert file name into heap")
            } /* end if */

            /* Store EFL file name offset */
            efl->slot[u].name_offset = offset;
        } /* end for */

        /* Release the heap */
        if(H5HL_unprotect(heap) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTUNPROTECT, FAIL, "unable to unprotect EFL file name heap")
        heap = NULL;

        /* Insert EFL message into dataset object header */
        if(H5O_msg_append_oh(file, dxpl_id, oh, H5O_EFL_ID, H5O_MSG_FLAG_CONSTANT, 0, efl) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to update external file list message")
    } /* end if */

    /* Create layout message */
    /* (Don't make layout message constant unless allocation time is early, since space may not be allocated) */
    /* (Note: this is relying on H5D_alloc_storage not calling H5O_msg_write during dataset creation) */
    if(H5O_msg_append_oh(file, dxpl_id, oh, H5O_LAYOUT_ID, ((fill_prop->alloc_time == H5D_ALLOC_TIME_EARLY && H5D_COMPACT != layout->type) ? H5O_MSG_FLAG_CONSTANT : 0), 0, layout) < 0)
         HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to update layout")

done:
    /* Error cleanup */
    if(ret_value < 0)
        if(layout_init)
            /* Destroy any cached layout information for the dataset */
            if(dset->shared->layout.ops->dest && (dset->shared->layout.ops->dest)(dset, dxpl_id) < 0)
                HDONE_ERROR(H5E_DATASET, H5E_CANTRELEASE, FAIL, "unable to destroy layout info")

    FUNC_LEAVE_NOAPI_TAG(ret_value, FAIL)
} /* end H5D__layout_oh_create() */


/*-------------------------------------------------------------------------
 * Function:	H5D__layout_oh_read
 *
 * Purpose:	Read layout/pline/efl information for dataset
 *
 * Return:	Success:    SUCCEED
 *		Failure:    FAIL
 *
 * Programmer:	Quincey Koziol
 *		Monday, July 27, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D__layout_oh_read(H5D_t *dataset, hid_t dxpl_id, hid_t dapl_id, H5P_genplist_t *plist)
{
    htri_t msg_exists;                  /* Whether a particular type of message exists */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_PACKAGE

    /* Sanity checking */
    HDassert(dataset);
    HDassert(plist);

    /* Get the optional filters message */
    if((msg_exists = H5O_msg_exists(&(dataset->oloc), H5O_PLINE_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't check if message exists")
    if(msg_exists) {
        /* Retrieve the I/O pipeline message */
        if(NULL == H5O_msg_read(&(dataset->oloc), H5O_PLINE_ID, &dataset->shared->dcpl_cache.pline, dxpl_id))
            HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't retrieve message")

        /* Set the I/O pipeline info in the property list */
        if(H5P_set(plist, H5O_CRT_PIPELINE_NAME, &dataset->shared->dcpl_cache.pline) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set pipeline")
    } /* end if */

    /*
     * Get the raw data layout info.  It's actually stored in two locations:
     * the storage message of the dataset (dataset->storage) and certain
     * values are copied to the dataset create plist so the user can query
     * them.
     */
    if(NULL == H5O_msg_read(&(dataset->oloc), H5O_LAYOUT_ID, &(dataset->shared->layout), dxpl_id))
        HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to read data layout message")

    /* Check for external file list message (which might not exist) */
    if((msg_exists = H5O_msg_exists(&(dataset->oloc), H5O_EFL_ID, dxpl_id)) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't check if message exists")
    if(msg_exists) {
        /* Retrieve the EFL  message */
        if(NULL == H5O_msg_read(&(dataset->oloc), H5O_EFL_ID, &dataset->shared->dcpl_cache.efl, dxpl_id))
            HGOTO_ERROR(H5E_DATASET, H5E_CANTGET, FAIL, "can't retrieve message")

        /* Set the EFL info in the property list */
        if(H5P_set(plist, H5D_CRT_EXT_FILE_LIST_NAME, &dataset->shared->dcpl_cache.efl) < 0)
            HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set external file list")

        /* Set the dataset's I/O operations */
        dataset->shared->layout.ops = H5D_LOPS_EFL;
    } /* end if */

    /* Sanity check that the layout operations are set up */
    HDassert(dataset->shared->layout.ops);

    /* Adjust chunk dimensions to omit datatype size (in last dimension) for creation property */
    if(H5D_CHUNKED == dataset->shared->layout.type)
        dataset->shared->layout.u.chunk.ndims--;
    /* Copy layout to the DCPL */
    if(H5P_set(plist, H5D_CRT_LAYOUT_NAME, &dataset->shared->layout) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "can't set layout")
    /* Adjust chunk dimensions back again (*sigh*) */
    if(H5D_CHUNKED == dataset->shared->layout.type)
        dataset->shared->layout.u.chunk.ndims++;

    /* Initialize the layout information for the dataset */
    if(dataset->shared->layout.ops->init && (dataset->shared->layout.ops->init)(dataset->oloc.file, dxpl_id, dataset, dapl_id) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTINIT, FAIL, "unable to initialize layout information")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D__layout_oh_read() */


/*-------------------------------------------------------------------------
 * Function:	H5D__layout_oh_write
 *
 * Purpose:	Write layout/pline/efl information for dataset
 *
 * Return:	Success:    SUCCEED
 *		Failure:    FAIL
 *
 * Programmer:	Quincey Koziol
 *		Monday, July 27, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5D__layout_oh_write(H5D_t *dataset, hid_t dxpl_id, H5O_t *oh, unsigned update_flags)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_PACKAGE

    /* Sanity checking */
    HDassert(dataset);
    HDassert(oh);

    /* Write the layout message to the dataset's header */
    if(H5O_msg_write_oh(dataset->oloc.file, dxpl_id, oh, H5O_LAYOUT_ID, H5O_MSG_FLAG_CONSTANT, update_flags, &dataset->shared->layout) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_WRITEERROR, FAIL, "unable to update layout message")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5D__layout_oh_write() */

