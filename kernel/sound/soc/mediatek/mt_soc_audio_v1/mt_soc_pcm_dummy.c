/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*******************************************************************************
 *
 * Filename:
 * ---------
 *   mt6583.c
 *
 * Project:
 * --------
 *   MT6583  Audio Driver Kernel Function
 *
 * Description:
 * ------------
 *   Audio register
 *
 * Author:
 * -------
 * Chipeng Chang
 *
 *------------------------------------------------------------------------------
 * $Revision: #1 $
 * $Modtime:$
 * $Log:$
 *
 *
 *******************************************************************************/


/*****************************************************************************
 *                     C O M P I L E R   F L A G S
 *****************************************************************************/


/*****************************************************************************
 *                E X T E R N A L   R E F E R E N C E S
 *****************************************************************************/

#include "AudDrv_Common.h"
#include "AudDrv_Def.h"
#include "AudDrv_Afe.h"
#include "AudDrv_Ana.h"
#include "AudDrv_Clk.h"
#include "mt_soc_pcm_common.h"

/*
 *    function implementation
 */

static int mtk_dummy_probe(struct platform_device *pdev);
static int mtk_dummypcm_close(struct snd_pcm_substream *substream);
static int mtk_asoc_dummypcm_new(struct snd_soc_pcm_runtime *rtd);
static int mtk_afe_dummy_probe(struct snd_soc_platform *platform);


static struct snd_soc_pcm_runtime *pruntimepcm = NULL;

static struct snd_pcm_hw_constraint_list constraints_sample_rates =
{
    .count = ARRAY_SIZE(soc_high_supported_sample_rates),
    .list = soc_high_supported_sample_rates,
    .mask = 0,
};

static int mtk_pcm_open(struct snd_pcm_substream *substream)
{

    struct snd_pcm_runtime *runtime = substream->runtime;

    int err = 0;
    int ret = 0;
    pr_debug("mtk_pcm_open\n");

    ret = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_RATE,
                                     &constraints_sample_rates);
    ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);


    if (ret < 0)
    {
        printk("snd_pcm_hw_constraint_integer failed\n");
    }

    //print for hw pcm information
    pr_debug("mtk_pcm_open runtime rate = %d channels = %d \n", runtime->rate, runtime->channels);
    if (substream->pcm->device & 1)
    {
        runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
        runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
    }
    if (substream->pcm->device & 2)
        runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP |
                              SNDRV_PCM_INFO_MMAP_VALID);

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        pr_debug("SNDRV_PCM_STREAM_PLAYBACK mtkalsa_playback_constraints\n");
    }
    else
    {

    }

    if (err < 0)
    {
        pr_debug("mtk_dummypcm_close\n");
        mtk_dummypcm_close(substream);
        return err;
    }
    pr_debug("mtk_pcm_open return\n");
    return 0;
}

static int mtk_dummypcm_close(struct snd_pcm_substream *substream)
{
    return 0;
}

static int mtk_dummypcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
    pr_debug("dummy_pcm_trigger cmd = %d\n", cmd);

    switch (cmd)
    {
        case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_STOP:
        case SNDRV_PCM_TRIGGER_SUSPEND:
            break;
    }
    return -EINVAL;
}

static int mtk_pcm_copy(struct snd_pcm_substream *substream,
                        int channel, snd_pcm_uframes_t pos,
                        void __user *dst, snd_pcm_uframes_t count)
{
    pr_debug("dummy_pcm_copy pos = %lu count = %lu \n ", pos, count);
    return 0;
}

static int mtk_pcm_silence(struct snd_pcm_substream *substream,
                           int channel, snd_pcm_uframes_t pos,
                           snd_pcm_uframes_t count)
{
    pr_debug("dummy_pcm_silence \n");
    return 0; /* do nothing */
}


static void *dummy_page[2];

static struct page *mtk_pcm_page(struct snd_pcm_substream *substream,
                                 unsigned long offset)
{
    pr_debug("dummy_pcm_page \n");
    return virt_to_page(dummy_page[substream->stream]); /* the same page */
}

static int mtk_pcm_prepare(struct snd_pcm_substream *substream)
{
    pr_debug("mtk_alsa_prepare \n");
    return 0;
}

static int mtk_pcm_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *hw_params)
{
    int ret = 0;
    PRINTK_AUDDRV("mtk_pcm_hw_params \n");
    return ret;
}

static int mtk_dummy_pcm_hw_free(struct snd_pcm_substream *substream)
{
    PRINTK_AUDDRV("mtk_dummy_pcm_hw_free \n");
    return snd_pcm_lib_free_pages(substream);
}

static struct snd_pcm_ops mtk_afe_ops =
{
    .open =     mtk_pcm_open,
    .close =    mtk_dummypcm_close,
    .ioctl =    snd_pcm_lib_ioctl,
    .hw_params =    mtk_pcm_hw_params,
    .hw_free =  mtk_dummy_pcm_hw_free,
    .prepare =  mtk_pcm_prepare,
    .trigger =  mtk_dummypcm_trigger,
    .copy =     mtk_pcm_copy,
    .silence =  mtk_pcm_silence,
    .page =     mtk_pcm_page,
};

static struct snd_soc_platform_driver mtk_soc_dummy_platform =
{
    .ops        = &mtk_afe_ops,
    .pcm_new    = mtk_asoc_dummypcm_new,
    .probe      = mtk_afe_dummy_probe,
};

static int mtk_dummy_probe(struct platform_device *pdev)
{
    pr_debug("mtk_dummy_probe\n");
    if (pdev->dev.of_node)
    {
        dev_set_name(&pdev->dev, "%s", MT_SOC_DUMMY_PCM);
    }

    pr_debug("%s: dev name %s\n", __func__, dev_name(&pdev->dev));
    return snd_soc_register_platform(&pdev->dev,
                                     &mtk_soc_dummy_platform);
}

static int mtk_asoc_dummypcm_new(struct snd_soc_pcm_runtime *rtd)
{
    int ret = 0;
    pruntimepcm  = rtd;
    pr_debug("%s\n", __func__);
    return ret;
}

static int mtk_afe_dummy_probe(struct snd_soc_platform *platform)
{
    pr_debug("mtk_afe_dummy_probe\n");
    return 0;
}


static int mtk_afedummy_remove(struct platform_device *pdev)
{
    pr_debug("%s\n", __func__);
    snd_soc_unregister_platform(&pdev->dev);
    return 0;
}

static struct platform_driver mtk_afedummy_driver =
{
    .driver = {
        .name = MT_SOC_DUMMY_PCM,
        .owner = THIS_MODULE,
    },
    .probe = mtk_dummy_probe,
    .remove = mtk_afedummy_remove,
};

static struct platform_device *soc_mtkafe_dummy_dev;

static int __init mtk_soc_dummy_platform_init(void)
{
    int ret =0;
    pr_debug("%s\n", __func__);

    soc_mtkafe_dummy_dev = platform_device_alloc(MT_SOC_DUMMY_PCM , -1);
    if (!soc_mtkafe_dummy_dev)
    {
        return -ENOMEM;
    }

    ret = platform_device_add(soc_mtkafe_dummy_dev);
    if (ret != 0)
    {
        platform_device_put(soc_mtkafe_dummy_dev);
        return ret;
    }

    ret = platform_driver_register(&mtk_afedummy_driver);

    return ret;

}
module_init(mtk_soc_dummy_platform_init);

static void __exit mtk_soc_dummy_platform_exit(void)
{

    pr_debug("%s\n", __func__);
    platform_driver_unregister(&mtk_afedummy_driver);
}
module_exit(mtk_soc_dummy_platform_exit);

MODULE_DESCRIPTION("AFE PCM module platform driver");
MODULE_LICENSE("GPL");


